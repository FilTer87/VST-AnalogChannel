/*
  ==============================================================================

    ToTape8.h
    Faithful port from AirWindows ToTape8

    Original Author: Chris Johnson (Airwindows)
    Original Source: Docs/Plugins_source_code/ToTape8/
    License: MIT

    Port to C++/JUCE: KuramaSound
    Date: 2024-11-06

    FAITHFUL PORT with FIXED parameters.
    All 9 parameters fixed at neutral/custom values except inputGain (Drive).

    Fixed parameters:
    - B (Dubly): 0.5
    - C (iirFreq): 0.5
    - D (Flutter depth): 0.38 (custom - reduced)
    - E (Flutter speed): 0.435 (custom - reduced)
    - F (Bias): 0.5
    - G (HeadBump): 0.5
    - H (HeadFreq): 68.75 Hz
    - I (OutputGain): 0.5

    Controlled by Drive:
    - A (inputGain)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    ToTape8 tape saturation algorithm from AirWindows.
    Full implementation with fixed parameters except inputGain.
*/
class ToTape8
{
public:
    enum
    {
        prevSamp1, threshold1,
        prevSamp2, threshold2,
        prevSamp3, threshold3,
        prevSamp4, threshold4,
        prevSamp5, threshold5,
        prevSamp6, threshold6,
        prevSamp7, threshold7,
        prevSamp8, threshold8,
        prevSamp9, threshold9,
        gslew_total
    };

    enum
    {
        hdb_freq, hdb_reso,
        hdb_a0, hdb_a1, hdb_a2,
        hdb_b1, hdb_b2,
        hdb_s1, hdb_s2,
        hdb_total
    };

    ToTape8()
    {
        reset();
    }

    //==============================================================================
    void setPRNGSeed (uint32_t seed)
    {
        fpd = seed;
        // Re-initialize nextmax with new seed
        fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
        nextmax = 0.24 + (fpd / static_cast<double>(UINT32_MAX) * 0.74);
        fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
        phantomNextmax = 0.24 + (fpd / static_cast<double>(UINT32_MAX) * 0.74);
    }

    void reset()
    {
        iirEnc = compEnc = avgEnc = 0.0;
        iirDec = compDec = avgDec = 0.0;

        for (int i = 0; i < 1002; ++i)
            delayBuffer[i] = 0.0;
        gcount = 0;
        sweep = 0.0;
        phantomSweep = 3.14159265358979323846;  // π radians offset

        // CRITICAL: Initialize nextmax with random values (0.24 to 0.98 range)
        // If nextmax = 0.0, sweep never increments and flutter is completely frozen!
        fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
        nextmax = 0.24 + (fpd / static_cast<double>(UINT32_MAX) * 0.74);
        fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
        phantomNextmax = 0.24 + (fpd / static_cast<double>(UINT32_MAX) * 0.74);

        for (int i = 0; i < gslew_total; ++i)
            gslew[i] = 0.0;

        iirMidRoller = iirLowCutoff = 0.0;
        headBump = 0.0;

        for (int i = 0; i < hdb_total; ++i)
        {
            hdbA[i] = hdbB[i] = 0.0;
        }

        lastSample = 0.0;
        for (int i = 0; i < 16; ++i)
            intermediate[i] = 0.0;
        wasPosClip = wasNegClip = false;

        fpd = 17;
    }

    void setSampleRate (double sampleRate)
    {
        currentSampleRate = sampleRate;

        overallscale = 1.0 / 44100.0 * currentSampleRate;
        spacing = static_cast<int> (std::floor (overallscale));
        if (spacing < 1) spacing = 1;
        if (spacing > 16) spacing = 16;

        // Fixed parameters
        double B = 0.5;   // Dubly
        double C = 0.5;   // iirFreq
        double D = 0.38;  // Flutter depth (custom - reduced)
        double E = 0.435; // Flutter speed (custom - reduced)
        double F = 0.5;   // Bias
        double G = 0.5;   // HeadBump
        double H = 0.5;   // HeadFreq (68.75 Hz with formula: ((H*H)*175+25))

        dublyAmount = B * 2.0;
        outlyAmount = (1.0 - B) * -2.0;
        if (outlyAmount < -1.0) outlyAmount = -1.0;

        iirEncFreq = (1.0 - C) / overallscale;
        iirDecFreq = C / overallscale;
        iirMidFreq = ((C * 0.618) + 0.382) / overallscale;

        flutDepth = std::pow (D, 6.0) * overallscale * 50.0;
        if (flutDepth > 498.0) flutDepth = 498.0;
        flutFrequency = (0.02 * std::pow (E, 3.0)) / overallscale;

        bias = (F * 2.0) - 1.0;
        underBias = (std::pow (bias, 4.0) * 0.25) / overallscale;
        double overBias = std::pow (1.0 - bias, 3.0) / overallscale;
        if (bias > 0.0) underBias = 0.0;
        if (bias < 0.0) overBias = 1.0 / overallscale;

        // Golden ratio cascade for gslew thresholds
        gslew[threshold9] = overBias;
        for (int i = 8; i >= 1; --i)
        {
            overBias *= 1.618033988749894848204586;
            gslew[threshold1 + (i - 1) * 2] = overBias;
        }

        headBumpDrive = (G * 0.1) / overallscale;
        headBumpMix = G * 0.5;
        double subCurve = std::sin (G * juce::MathConstants<double>::pi);
        iirSubFreq = (subCurve * 0.008) / overallscale;

        hdbA[hdb_freq] = (((H * H) * 175.0) + 25.0) / currentSampleRate;
        hdbB[hdb_freq] = hdbA[hdb_freq] * 0.9375;
        hdbA[hdb_reso] = hdbB[hdb_reso] = 0.618033988749894848204586;
        hdbA[hdb_a1] = hdbB[hdb_a1] = 0.0;

        double K = std::tan (juce::MathConstants<double>::pi * hdbA[hdb_freq]);
        double norm = 1.0 / (1.0 + K / hdbA[hdb_reso] + K * K);
        hdbA[hdb_a0] = K / hdbA[hdb_reso] * norm;
        hdbA[hdb_a2] = -hdbA[hdb_a0];
        hdbA[hdb_b1] = 2.0 * (K * K - 1.0) * norm;
        hdbA[hdb_b2] = (1.0 - K / hdbA[hdb_reso] + K * K) * norm;

        K = std::tan (juce::MathConstants<double>::pi * hdbB[hdb_freq]);
        norm = 1.0 / (1.0 + K / hdbB[hdb_reso] + K * K);
        hdbB[hdb_a0] = K / hdbB[hdb_reso] * norm;
        hdbB[hdb_a2] = -hdbB[hdb_a0];
        hdbB[hdb_b1] = 2.0 * (K * K - 1.0) * norm;
        hdbB[hdb_b2] = (1.0 - K / hdbB[hdb_reso] + K * K) * norm;

        double I = 0.5; // Output gain fixed at 0.5
        outputGain = I * 2.0;
    }

    //==============================================================================
    float process (float input, float driveDB)
    {
        double inputSample = input;
        if (std::fabs (inputSample) < 1.18e-23)
            inputSample = fpd * 1.18e-17;

        // New behavior: negative drive = volume only, positive drive = algorithm drive
        double A;
        if (driveDB < 0.0)
        {
            // Negative drive: just attenuate volume, keep algorithm at neutral (A = 0.5)
            double volumeGain = std::pow (10.0, driveDB / 20.0);
            inputSample *= volumeGain;
            A = 0.5;  // Algorithm stays neutral
        }
        else
        {
            // Positive drive: map 0..+18 dB to A 0.5..1.0
            A = 0.5 + (driveDB / 36.0);  // 0 dB → 0.5, +18 dB → 1.0
            A = juce::jlimit (0.5, 1.0, A);

            // Apply input gain for algorithm
            double inputGain = std::pow (A * 2.0, 2.0);
            if (inputGain != 1.0)
                inputSample *= inputGain;
        }

        // Dubly encode
        iirEnc = (iirEnc * (1.0 - iirEncFreq)) + (inputSample * iirEncFreq);
        double highPart = ((inputSample - iirEnc) * 2.848);
        highPart += avgEnc;
        avgEnc = (inputSample - iirEnc) * 1.152;
        if (highPart > 1.0) highPart = 1.0;
        if (highPart < -1.0) highPart = -1.0;
        double dubly = std::fabs (highPart);
        if (dubly > 0.0)
        {
            double adjust = std::log (1.0 + (255.0 * dubly)) / 2.40823996531;
            if (adjust > 0.0) dubly /= adjust;
            compEnc = (compEnc * (1.0 - iirEncFreq)) + (dubly * iirEncFreq);
            inputSample += ((highPart * compEnc) * dublyAmount);
        }

        // Flutter
        if (flutDepth > 0.0)
        {
            if (gcount < 0 || gcount > 999) gcount = 999;
            delayBuffer[gcount] = inputSample;
            int count = gcount;
            double offset = flutDepth + (flutDepth * std::sin (sweep));
            sweep += nextmax * flutFrequency;

            // Update phantom channel sweep (mimics opposite channel in stereo)
            phantomSweep += phantomNextmax * flutFrequency;

            if (sweep > (juce::MathConstants<double>::twoPi))
            {
                sweep -= juce::MathConstants<double>::twoPi;
                fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
                double flutA = 0.24 + (fpd / static_cast<double>(UINT32_MAX) * 0.74);
                fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
                double flutB = 0.24 + (fpd / static_cast<double>(UINT32_MAX) * 0.74);
                // Cross-coupling: compare against phantom channel (mimics L uses R, R uses L in stereo)
                if (std::fabs (flutA - std::sin (phantomSweep + phantomNextmax)) < std::fabs (flutB - std::sin (phantomSweep + phantomNextmax)))
                    nextmax = flutA;
                else
                    nextmax = flutB;
            }

            // Handle phantom channel wrap and random generation
            if (phantomSweep > (juce::MathConstants<double>::twoPi))
            {
                phantomSweep -= juce::MathConstants<double>::twoPi;
                fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
                double phantomFlutA = 0.24 + (fpd / static_cast<double>(UINT32_MAX) * 0.74);
                fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
                double phantomFlutB = 0.24 + (fpd / static_cast<double>(UINT32_MAX) * 0.74);
                // Phantom channel compares against main channel (opposite direction)
                if (std::fabs (phantomFlutA - std::sin (sweep + nextmax)) < std::fabs (phantomFlutB - std::sin (sweep + nextmax)))
                    phantomNextmax = phantomFlutA;
                else
                    phantomNextmax = phantomFlutB;
            }
            count += static_cast<int> (std::floor (offset));
            inputSample = (delayBuffer[count - ((count > 999) ? 1000 : 0)] * (1.0 - (offset - std::floor (offset))));
            inputSample += (delayBuffer[count + 1 - ((count + 1 > 999) ? 1000 : 0)] * (offset - std::floor (offset)));
            gcount--;
        }

        // Bias routine
        // In the original stereo code: gslew[x]=prevSampL, gslew[x+1]=prevSampR, gslew[x+2]=threshold
        // In mono: gslew[x]=prevSamp, gslew[x+1]=threshold (no x+2!)
        if (std::fabs (bias) > 0.001)
        {
            for (int x = 0; x < gslew_total; x += 2)
            {
                double currentThreshold = gslew[x + 1];  // threshold is at x+1 for mono

                if (underBias > 0.0)
                {
                    double stuck = std::fabs (inputSample - (gslew[x] / 0.975)) / underBias;
                    if (stuck < 1.0)
                        inputSample = (inputSample * stuck) + ((gslew[x] / 0.975) * (1.0 - stuck));
                }

                // Use currentThreshold instead of gslew[x+2] (which doesn't exist in mono!)
                if ((inputSample - gslew[x]) > currentThreshold)
                    inputSample = gslew[x] + currentThreshold;
                if (-(inputSample - gslew[x]) > currentThreshold)
                    inputSample = gslew[x] - currentThreshold;

                gslew[x] = inputSample * 0.975;
            }
        }

        // toTape basic algorithm
        iirMidRoller = (iirMidRoller * (1.0 - iirMidFreq)) + (inputSample * iirMidFreq);
        double HighsSample = inputSample - iirMidRoller;
        double LowsSample = iirMidRoller;

        if (iirSubFreq > 0.0)
        {
            iirLowCutoff = (iirLowCutoff * (1.0 - iirSubFreq)) + (LowsSample * iirSubFreq);
            LowsSample -= iirLowCutoff;
        }

        if (LowsSample > 1.57079633) LowsSample = 1.57079633;
        if (LowsSample < -1.57079633) LowsSample = -1.57079633;
        LowsSample = std::sin (LowsSample);

        double thinnedHighSample = std::fabs (HighsSample) * 1.57079633;
        if (thinnedHighSample > 1.57079633) thinnedHighSample = 1.57079633;
        thinnedHighSample = 1.0 - std::cos (thinnedHighSample);
        if (HighsSample < 0) thinnedHighSample = -thinnedHighSample;
        HighsSample -= thinnedHighSample;

        // HeadBump
        double headBumpSample = 0.0;
        if (headBumpMix > 0.0)
        {
            headBump += (LowsSample * headBumpDrive);
            headBump -= (headBump * headBump * headBump * (0.0618 / std::sqrt (overallscale)));
            double headBiqSample = (headBump * hdbA[hdb_a0]) + hdbA[hdb_s1];
            hdbA[hdb_s1] = (headBump * hdbA[hdb_a1]) - (headBiqSample * hdbA[hdb_b1]) + hdbA[hdb_s2];
            hdbA[hdb_s2] = (headBump * hdbA[hdb_a2]) - (headBiqSample * hdbA[hdb_b2]);
            headBumpSample = (headBiqSample * hdbB[hdb_a0]) + hdbB[hdb_s1];
            hdbB[hdb_s1] = (headBiqSample * hdbB[hdb_a1]) - (headBumpSample * hdbB[hdb_b1]) + hdbB[hdb_s2];
            hdbB[hdb_s2] = (headBiqSample * hdbB[hdb_a2]) - (headBumpSample * hdbB[hdb_b2]);
        }

        inputSample = LowsSample + HighsSample + (headBumpSample * headBumpMix);

        // Dubly decode
        iirDec = (iirDec * (1.0 - iirDecFreq)) + (inputSample * iirDecFreq);
        highPart = ((inputSample - iirDec) * 2.628);
        highPart += avgDec;
        avgDec = (inputSample - iirDec) * 1.372;
        if (highPart > 1.0) highPart = 1.0;
        if (highPart < -1.0) highPart = -1.0;
        dubly = std::fabs (highPart);
        if (dubly > 0.0)
        {
            double adjust = std::log (1.0 + (255.0 * dubly)) / 2.40823996531;
            if (adjust > 0.0) dubly /= adjust;
            compDec = (compDec * (1.0 - iirDecFreq)) + (dubly * iirDecFreq);
            inputSample += ((highPart * compDec) * outlyAmount);
        }

        if (outputGain != 1.0)
            inputSample *= outputGain;

        // ClipOnly2
        if (inputSample > 4.0) inputSample = 4.0;
        if (inputSample < -4.0) inputSample = -4.0;

        if (wasPosClip)
        {
            if (inputSample < lastSample)
                lastSample = 0.7058208 + (inputSample * 0.2609148);
            else
                lastSample = 0.2491717 + (lastSample * 0.7390851);
        }
        wasPosClip = false;
        if (inputSample > 0.9549925859)
        {
            wasPosClip = true;
            inputSample = 0.7058208 + (lastSample * 0.2609148);
        }

        if (wasNegClip)
        {
            if (inputSample > lastSample)
                lastSample = -0.7058208 + (inputSample * 0.2609148);
            else
                lastSample = -0.2491717 + (lastSample * 0.7390851);
        }
        wasNegClip = false;
        if (inputSample < -0.9549925859)
        {
            wasNegClip = true;
            inputSample = -0.7058208 + (lastSample * 0.2609148);
        }

        intermediate[spacing] = inputSample;
        inputSample = lastSample;
        for (int x = spacing; x > 0; x--)
            intermediate[x - 1] = intermediate[x];
        lastSample = intermediate[0];

        return static_cast<float> (inputSample);
    }

private:
    //==============================================================================
    double currentSampleRate = 44100.0;
    double overallscale = 1.0;
    int spacing = 1;

    // Fixed parameters (set in setSampleRate)
    double dublyAmount, outlyAmount;
    double iirEncFreq, iirDecFreq, iirMidFreq;
    double flutDepth, flutFrequency;
    double bias, underBias;  // Bias routine parameters
    double headBumpDrive, headBumpMix, iirSubFreq;
    double outputGain;

    // Dubly state
    double iirEnc, iirDec;
    double compEnc, compDec;
    double avgEnc, avgDec;

    // Flutter state
    double delayBuffer[1002];  // 1002 samples (not 1000) to prevent overflow when accessing count+1
    int gcount;
    double sweep, nextmax;
    double phantomSweep, phantomNextmax;  // Phantom channel for cross-coupling (mimics stereo behavior)

    // Bias state
    double gslew[gslew_total];

    // toTape state
    double iirMidRoller, iirLowCutoff;

    // HeadBump state
    double headBump;
    double hdbA[hdb_total];
    double hdbB[hdb_total];

    // ClipOnly2 state
    double lastSample;
    double intermediate[16];
    bool wasPosClip, wasNegClip;

    // PRNG
    uint32_t fpd;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToTape8)
};
