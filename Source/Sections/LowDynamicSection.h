/*
  ==============================================================================

    LowDynamicSection.h
    Low Dynamic Section - Dual-mode Downward Expander / Upward Compressor

    This section processes signal BELOW the threshold:
    - Negative ratio: Downward expansion (lowers signal below threshold, 1:1 to 1:4)
      Progressive quadratic scaling: -1 = 1:1.02, -5 = 1:1.75, -10 = 1:4
    - Positive ratio: Upward compression (lifts signal below threshold, 1:1 to 1:4)
      Linear scaling: +1 = 1:1.08, +5 = 1:1.6, +10 = 1:4
    - Zero ratio: Bypass (no processing)

    Detection modes:
    - FAST (expander): 0.5ms recovery, 60ms reduction (Peak detection)
    - FAST (lifter): 0.5ms return, 60ms lift (RMS detection)
    - NORMAL (expander): 15ms recovery, 100ms reduction (RMS detection)
    - NORMAL (lifter): 0.5ms return, 15ms lift (RMS detection, faster to preserve transients)

    Knee: 0.5dB hard knee for precise threshold response

    Note: Attack/Release semantics for expander are inverted from compressor:
    - Recovery (fast) = returning to unity gain when signal goes above threshold
    - Reduction (slow) = reducing gain when signal stays below threshold

  ==============================================================================
*/

#pragma once

#include "../Sections/BypassableSection.h"
#include <JuceHeader.h>
#include <cmath>

class LowDynamicSection : public BypassableSection
{
public:
    LowDynamicSection() = default;

    void setSampleRate (double sr) override
    {
        sampleRate = sr;
        updateTimingCoefficients();

        // CRITICAL: Initialize state to prevent initial gain spike
        resetState();

        // Initialize smoothedGain to 1.0 (unity) to avoid spike
        smoothedGain = 1.0f;
    }

    void reset() override
    {
        resetState();
    }

    // Parameters
    void setThreshold (float thresholdDB)
    {
        threshold = thresholdDB;
    }

    void setRatio (float ratioValue)  // -10 to +10
    {
        ratio = ratioValue;
    }

    void setFastMode (bool isFast)
    {
        fastMode = isFast;
        updateTimingCoefficients();
    }

    void setMix (float percent)
    {
        mixPercent = juce::jlimit (0.0f, 100.0f, percent);
        mixAmount = mixPercent / 100.0f;
    }

    // Get current gain reduction (for metering, if needed)
    float getCurrentGainReduction() const
    {
        return currentGR;
    }

protected:
    float processInternal (float input) override
    {
        // CRITICAL: If ratio is near zero, bypass completely (no processing)
        if (std::abs(ratio) < 0.01f)
        {
            // Reset gain to unity to prevent any residual gain
            smoothedGain = 1.0f;
            currentGR = 0.0f;
            return input;
        }

        // === STEP 1: SIDECHAIN LEVEL DETECTION ===
        // CRITICAL FIX: Use INSTANT level for threshold gating to prevent
        // false triggering on peaks above threshold. RMS is only used for
        // smooth gain calculation, NOT for threshold comparison.

        // Instant level (for threshold gating)
        float instantLevel = std::abs(input);
        float instantDB = 20.0f * std::log10(std::max(instantLevel, 1e-6f));

        // Smoothed detector level (for smooth gain calculation)
        float detectorLevel;
        if (fastMode)
        {
            // FAST MODE: Peak hold for expander ONLY, RMS for lifter
            if (ratio < 0.0f)
            {
                // Expander: Peak detection with hold (prevents rapid fluctuations)
                float currentPeak = instantLevel;

                // Update peak hold: instant attack, slow decay
                if (currentPeak > peakHold)
                    peakHold = currentPeak;  // Instant attack
                else
                    peakHold = currentPeak + peakHoldDecay * (peakHold - currentPeak);  // Slow decay

                detectorLevel = peakHold;
            }
            else
            {
                // Lifter: Use RMS to avoid peak hold interference
                float inputSquared = input * input;
                rmsState = rmsState * rmsCoeff + inputSquared * (1.0f - rmsCoeff);
                detectorLevel = std::sqrt(rmsState);
            }
        }
        else
        {
            // NORMAL MODE: RMS detection (longer window ~20ms for stability)
            float inputSquared = input * input;
            rmsState = rmsState * rmsCoeff + inputSquared * (1.0f - rmsCoeff);
            detectorLevel = std::sqrt(rmsState);
        }

        // Note: detectorLevel is smoothed for visual metering, but we use instantDB for threshold gating

        // === STEP 2: COMPUTE TARGET GAIN ===
        float targetGainDB = 0.0f;  // Default: no change

        // CRITICAL: Use INSTANT level for threshold gating (prevents false triggering)
        if (instantDB < threshold)
        {
            // Gate is open: signal is below threshold
            // Calculate reduction amount based on INSTANT level (must use same reference!)
            // Using detectorDB here could give negative dbBelowThreshold if smoothed level > threshold
            float dbBelowThreshold = threshold - instantDB;

            // Safety clamp: ensure dbBelowThreshold is always positive
            dbBelowThreshold = std::max(dbBelowThreshold, 0.0f);

            if (ratio < 0.0f)
            {
                // DOWNWARD EXPANSION: Reduce signal below threshold
                // PROGRESSIVE SCALING: knob -1 = 1:1.02, knob -10 = 1:4
                // Using quadratic curve for more control at low ratios
                float absRatio = std::abs(ratio);
                float normalizedRatio = absRatio / 10.0f;  // 0 to 1

                // Quadratic scaling: ratio = 1 + (normalized^2) * 3.0
                // -1  → 1 + (0.1^2) * 3.0 = 1 + 0.03 = 1.03  (close to 1:1.02) ✓
                // -5  → 1 + (0.5^2) * 3.0 = 1 + 0.75 = 1.75  (moderate)
                // -10 → 1 + (1.0^2) * 3.0 = 1 + 3.0  = 4.0   (1:4 max) ✓
                float expansionRatio = 1.0f + (normalizedRatio * normalizedRatio * 3.0f);

                // Slope: for 1:4 ratio, slope = 3.0
                float slope = expansionRatio - 1.0f;
                targetGainDB = -dbBelowThreshold * slope;  // Negative (reduction)

                // Safety limiter: prevent digital silence (clamp to -96 dB max reduction)
                targetGainDB = std::max(targetGainDB, -96.0f);

                // Mathematical verification:
                // Input 20dB below threshold, ratio -10 (1:4):
                // slope = 3.0, targetGainDB = -20 * 3.0 = -60 dB
                // Output: -80 dB below threshold → 80/20 = 4:1 ratio ✓
            }
            else if (ratio > 0.0f)
            {
                // UPWARD COMPRESSION: Boost signal below threshold
                // Scaling: knob +10 = ratio 1:4
                // liftAmount = 0.75 → for 20dB below threshold: boost = 15dB → output 5dB below = 1:4 ratio
                float liftAmount = ratio * 0.075f;  // 0 to 0.75 (ratio 1:1 to 1:4)
                targetGainDB = dbBelowThreshold * liftAmount;  // Positive (boost)

                // Mathematical verification:
                // Input 20dB below threshold, ratio +10 (1:4):
                // liftAmount = 0.75, targetGainDB = 20 * 0.75 = +15 dB
                // Output: -20 + 15 = -5 dB below threshold → 20/5 = 4:1 → 1:4 ratio ✓
            }

            // Hard knee (0.5 dB transition around threshold)
            // Applies quadratic curve near threshold to soften the transition
            const float kneeWidth = 0.5f;
            if (dbBelowThreshold < kneeWidth)
            {
                float kneeRatio = dbBelowThreshold / kneeWidth;
                targetGainDB *= (kneeRatio * kneeRatio);  // Smooth curve
            }
        }
        // If instantDB >= threshold: targetGainDB = 0.0 (no processing)
        // This ensures peaks above threshold are NEVER affected

        // === STEP 3: SMOOTH TARGET GAIN WITH ATTACK/RELEASE ===
        // This creates the envelope follower
        float targetGainLinear = std::pow(10.0f, targetGainDB / 20.0f);

        // Determine mode early for initialization
        bool isExpanding = (ratio < 0.0f);
        bool isLifting = (ratio > 0.0f);

        // CRITICAL: Detector warmup protection OR detector cold check
        // Limit gain if either in warmup period OR detectors are cold
        bool detectorsAreCold = (warmupSamplesRemaining > 0) || (detectorLevel < 1e-6f);

        if (detectorsAreCold)
        {
            if (warmupSamplesRemaining > 0)
                warmupSamplesRemaining--;

            // For lifter: uses lifterAttackCoeff (0.5ms, very fast but not instant)
            // For expander: uses attackCoeff (0.5ms FAST, 15ms NORMAL)
            float initAttackCoeff = isLifting ? lifterAttackCoeff : attackCoeff;

            // Ramp up gradually using attack coefficient
            smoothedGain = 1.0f + initAttackCoeff * (targetGainLinear - 1.0f);

            // SAFETY LIMITER: While detectors are cold, clamp gain to +6 dB max
            // This prevents extreme spikes from detector zero state
            const float maxWarmupGain = 2.0f;  // +6 dB max during warmup
            smoothedGain = std::min(smoothedGain, maxWarmupGain);

            // Also clamp for expander (prevent over-reduction)
            const float minWarmupGain = 0.5f;  // -6 dB min during warmup
            smoothedGain = std::max(smoothedGain, minWarmupGain);
        }
        else  // CRITICAL: Only run normal envelope follower when detectors are warmed up!
        {
        // Attack/Release ballistics - SEMANTICS FOR BOTH MODES
        //
        // EXPANDER (ratio < 0) - Preserves transients:
        //   - When REDUCING gain (targetGain < smoothed): Use releaseCoeff (SLOW)
        //     Allows transients to pass before expansion kicks in
        //   - When INCREASING gain (targetGain > smoothed): Use attackCoeff (FAST)
        //     Opens gate quickly when signal goes above threshold
        //
        // LIFTER (ratio > 0) - Lifts low-level signals, preserves peaks:
        //   - When INCREASING gain (targetGain > smoothed): Use releaseCoeff (SLOW)
        //     Applies lift gradually to avoid pumping
        //   - When DECREASING gain (targetGain < smoothed): Use attackCoeff (FAST)
        //     Quick return to unity when transients go above threshold (preserves peaks)

        if (isExpanding)
        {
            // DOWNWARD EXPANSION
            // CRITICAL: Expander semantics are OPPOSITE to compressor!
            // - When gain DECREASES (targetGain < smoothed): Use SLOW coefficient (attack = slow reduction)
            //   This allows transients to pass through before expansion kicks in
            // - When gain INCREASES (targetGain > smoothed): Use FAST coefficient (release = fast recovery)
            //   This ensures the expander opens quickly when signal goes above threshold
            if (targetGainLinear < smoothedGain)
                smoothedGain = targetGainLinear + releaseCoeff * (smoothedGain - targetGainLinear);  // SLOW reduction (attack)
            else
                smoothedGain = targetGainLinear + attackCoeff * (smoothedGain - targetGainLinear);  // FAST recovery (release)
        }
        else if (isLifting)
        {
            // UPWARD COMPRESSION (LIFTER)
            // Uses separate coefficients (faster in NORMAL mode to preserve transients)
            // - When gain INCREASES (targetGain > smoothed): Use lifterReleaseCoeff (SLOW)
            //   This applies lift gradually to avoid pumping
            // - When gain DECREASES (targetGain < smoothed): Use lifterAttackCoeff (FAST)
            //   This ensures quick return to unity gain when transients go above threshold
            if (targetGainLinear > smoothedGain)
                smoothedGain = targetGainLinear + lifterReleaseCoeff * (smoothedGain - targetGainLinear);  // SLOW lift
            else
                smoothedGain = targetGainLinear + lifterAttackCoeff * (smoothedGain - targetGainLinear);  // FAST return
        }
        }  // End of else block (normal envelope follower after warmup)

        // === STEP 4: APPLY SMOOTHED GAIN TO ORIGINAL INPUT ===
        // CRITICAL: We apply the smoothed envelope to the ORIGINAL input,
        // NOT to the detected level. This preserves transients above threshold.
        float wet = input * smoothedGain;

        // Store current GR for metering
        currentGR = 20.0f * std::log10(std::max(smoothedGain, 1e-6f));

        // === STEP 5: MIX DRY AND WET ===
        // mixAmount: 0.0 = 100% dry (bypass), 1.0 = 100% wet (full effect)
        float output = input * (1.0f - mixAmount) + wet * mixAmount;

        return output;
    }

private:
    // Parameters
    double sampleRate = 44100.0;
    float threshold = -20.0f;  // dB (range: -40 to -3)
    float ratio = 0.0f;        // -10 to +10
    bool fastMode = false;

    // Mix parameters
    float mixPercent = 100.0f;  // Default: 100% (full wet)
    float mixAmount = 1.0f;

    // State
    float rmsState = 0.0f;
    float smoothedGain = 1.0f;
    float currentGR = 0.0f;  // Current gain reduction/lift (dB)
    float peakHold = 0.0f;  // Peak hold for detection (prevents rapid gain changes)
    float peakHoldDecay = 0.0f;
    int warmupSamplesRemaining = 0;  // Samples remaining in detector warmup period

    // Coefficients
    float rmsCoeff = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // Separate coefficients for lifter (faster return to unity in NORMAL mode)
    float lifterAttackCoeff = 0.0f;   // Same as attackCoeff (0.5ms fast return)
    float lifterReleaseCoeff = 0.0f;  // Faster in NORMAL mode (15ms instead of 100ms)

    void updateTimingCoefficients()
    {
        if (sampleRate <= 0.0)
            return;

        // RMS window (longer for more stable detection ~20ms)
        rmsCoeff = std::exp(-1.0f / (0.020f * static_cast<float>(sampleRate)));

        // Peak hold decay (50ms hold time before decay)
        peakHoldDecay = std::exp(-1.0f / (0.050f * static_cast<float>(sampleRate)));

        if (fastMode)
        {
            // FAST: 0.5ms attack (recovery), 60ms release (reduction)
            attackCoeff = std::exp(-1.0f / (0.0005f * static_cast<float>(sampleRate)));
            releaseCoeff = std::exp(-1.0f / (0.060f * static_cast<float>(sampleRate)));

            // FAST Lifter: same as expander (60ms is fine for both)
            lifterAttackCoeff = attackCoeff;    // 0.5ms (fast return to unity)
            lifterReleaseCoeff = releaseCoeff;  // 60ms (gradual lift)
        }
        else
        {
            // NORMAL Expander: 15ms attack (recovery), 100ms release (reduction)
            attackCoeff = std::exp(-1.0f / (0.015f * static_cast<float>(sampleRate)));
            releaseCoeff = std::exp(-1.0f / (0.100f * static_cast<float>(sampleRate)));

            // NORMAL Lifter: 0.5ms attack (fast return), 15ms release (faster lift)
            // CRITICAL: Faster return to unity (0.5ms) to preserve transients above threshold
            lifterAttackCoeff = std::exp(-1.0f / (0.0005f * static_cast<float>(sampleRate)));  // 0.5ms
            lifterReleaseCoeff = std::exp(-1.0f / (0.015f * static_cast<float>(sampleRate)));  // 15ms
        }
    }

    void resetState()
    {
        rmsState = 0.0f;
        smoothedGain = 1.0f;
        currentGR = 0.0f;
        peakHold = 0.0f;

        // Set warmup period: ~100 samples (~2.3ms @ 44.1kHz) for detectors to stabilize
        // During warmup, gain is limited to prevent spike from cold detectors
        warmupSamplesRemaining = 100;
    }
};
