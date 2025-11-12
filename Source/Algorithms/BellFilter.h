/*
  ==============================================================================

    BellFilter.h
    Custom Bell Filter with API 550-style Dynamic Q

    Implements a parametric bell filter where Q changes based on:
    - Gain amount (more gain = narrower Q)
    - Frequency (low freq = narrower, high freq = wider)

    This mimics the behavior of classic API 550 equalizers.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Bell filter with dynamic Q based on gain and frequency.
    API 550-style behavior.
*/
class BellFilter
{
public:
    BellFilter() = default;

    //==============================================================================
    void reset()
    {
        filter.reset();
    }

    void setSampleRate (double sampleRate)
    {
        currentSampleRate = sampleRate;
        updateCoefficients();
    }

    //==============================================================================
    /**
        Sets the bell filter parameters.
        @param freqHz frequency in Hz
        @param gainDB gain in dB (-12 to +12)
    */
    void setParameters (float freqHz, float gainDB)
    {
        currentFreq = freqHz;
        currentGain = gainDB;
        updateCoefficients();
    }

    /**
        Sets Q offset for channel variation.
        This offset is added to the dynamically calculated Q value.
        @param offset Q offset (typically ±0.06)
    */
    void setQOffset (float offset)
    {
        qOffset = offset;
        updateCoefficients();
    }

    /**
        Process a single sample.
    */
    float process (float input)
    {
        return filter.processSample (input);
    }

private:
    //==============================================================================
    /**
        Calculate dynamic Q based on gain and frequency (API 550-style).
    */
    float calculateDynamicQ (float gainDB, float freqHz)
    {
        float Q;
        float absGain = std::fabs (gainDB);

        // Base Q from gain amount
        if (gainDB > 0.0f)
        {
            // Positive gain: Q from 0.15 (1dB) to 0.75 (12dB)
            // More boost = narrower (higher Q)
            Q = 0.15f + (absGain / 12.0f) * (0.75f - 0.15f);
        }
        else
        {
            // Negative gain (cut): Q from 0.25 to 3.3
            // More cut = much narrower (higher Q)
            Q = 0.25f + (absGain / 12.0f) * (3.3f - 0.5f);
        }

        // Frequency compensation
        // Low frequencies: narrower (multiply by 1.1)
        // High frequencies: wider (multiply by 0.9)
        float freqFactor;

        if (freqHz < 500.0f)
        {
            freqFactor = 1.1f;  // Narrower at low frequencies
        }
        else if (freqHz > 3000.0f)
        {
            freqFactor = 0.9f;  // Wider at high frequencies
        }
        else
        {
            // Linear interpolation between 500-3000 Hz
            float t = (freqHz - 500.0f) / 2500.0f;
            freqFactor = 1.1f + t * (0.9f - 1.1f);
        }

        Q *= freqFactor;

        return Q;
    }

    void updateCoefficients()
    {
        if (currentSampleRate <= 0.0)
            return;

        // Calculate dynamic Q
        float Q = calculateDynamicQ (currentGain, currentFreq);

        // Apply channel variation Q offset
        Q += qOffset;

        // Clamp Q to reasonable range to avoid instability
        Q = juce::jlimit (0.1f, 10.0f, Q);

        // Convert dB to linear gain
        float linearGain = std::pow (10.0f, currentGain / 20.0f);

        // Limit frequency to valid range
        float limitedFreq = juce::jlimit (
            20.0f,
            static_cast<float>(currentSampleRate * 0.49),
            currentFreq
        );

        // Create peak filter coefficients
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            currentSampleRate,
            limitedFreq,
            Q,
            linearGain
        );

        filter.coefficients = coeffs;
    }

    //==============================================================================
    double currentSampleRate = 44100.0;
    float currentFreq = 1000.0f;
    float currentGain = 0.0f;
    float qOffset = 0.0f;  // Channel variation Q offset

    juce::dsp::IIR::Filter<float> filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BellFilter)
};
