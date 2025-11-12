/*
  ==============================================================================

    StyleCompSection.h
    Section 5: Style-Comp (Character Compression)
    Algorithms: Warm (CL1B optical) and Punch (Digital Versatile aggressive)

    Features:
    - Fixed threshold at -10dB for consistent compression behavior
    - Comp IN control: Pre/post gain staging (-18dB to +60dB)
    - Manual makeup gain control (-6dB to +24dB)
    - Gain reduction metering
    - Warm: CL1B optical 4:1 ratio (musical, adaptive)
    - Punch: Digital Versatile 20:1 ratio, 24ms attack (limiter-like)

  ==============================================================================
*/

#pragma once

#include "BypassableSection.h"
#include "../Algorithms/CL1BCompressor.h"
#include "../Algorithms/DigitalVersatileCompressor.h"

//==============================================================================
/**
    Style-Comp section - Character compression for musical coloration.
    Two modes: Warm (optical 6:1) and Punch (aggressive 7:1).
    Fixed threshold (-10dB) with Comp IN drive and manual Makeup controls.
*/
class StyleCompSection : public BypassableSection
{
public:
    enum Algorithm
    {
        Warm = 0,   // CL1B optical compressor (6:1)
        Punch = 1   // Digital Versatile aggressive (7:1)
    };

    StyleCompSection()
    {
        compInGain = 1.0f;
        makeupGain = 1.0f;
        mixAmount = 1.0f;  // Default 100% wet

        // Initialize both compressors with fixed threshold
        warmCompressor.setParameters (-10.0f);  // Ratio 4:1 fixed internally
        punchCompressor.setParameters (-10.0f, 20.0f, 24.0f, 10.0f);  // Ratio 20:1, Attack 24ms
    }

    //==============================================================================
    void setSampleRate (double sampleRate) override
    {
        BypassableSection::setSampleRate (sampleRate);
        warmCompressor.setSampleRate (sampleRate);
        punchCompressor.setSampleRate (sampleRate);
        updateCompression();
    }

    void reset() override
    {
        warmCompressor.reset();
        punchCompressor.reset();
    }

    //==============================================================================
    /**
        Sets the compression algorithm.
        @param algo Warm or Punch mode
    */
    void setAlgorithm (Algorithm algo)
    {
        currentAlgorithm = algo;
        updateCompression();
    }

    /**
        Sets the Comp IN amount in decibels.
        Applied as pre/post gain staging around compression.
        @param dB Comp IN from -18 dB to +60 dB
    */
    void setCompIn (float dB)
    {
        compInDB = juce::jlimit (-18.0f, 60.0f, dB);
        compInGain = std::pow (10.0f, compInDB / 20.0f);
    }

    /**
        Sets the makeup gain in decibels.
        Applied after compression.
        @param dB Makeup gain from -6 dB to +24 dB
    */
    void setMakeup (float dB)
    {
        makeupDB = juce::jlimit (-6.0f, 24.0f, dB);
        makeupGain = std::pow (10.0f, makeupDB / 20.0f);
    }

    /**
        Sets the dry/wet mix amount.
        @param percent Mix from 0% (dry) to 100% (wet)
    */
    void setMix (float percent)
    {
        mixPercent = juce::jlimit (0.0f, 100.0f, percent);
        mixAmount = mixPercent / 100.0f;
    }

    /**
        Get current gain reduction for metering.
        @return gain reduction in dB (negative value)
    */
    float getGainReductionDB() const
    {
        if (currentAlgorithm == Warm)
            return warmCompressor.getGainReductionDB();
        else
            return punchCompressor.getGainReductionDB();
    }

protected:
    //==============================================================================
    float processInternal (float input) override
    {
        // Store dry signal for mixing
        float dry = input;

        // Apply Comp IN gain (increase level before compression)
        float driven = input * compInGain;

        // Process through selected compressor
        float compressed;

        if (currentAlgorithm == Warm)
        {
            compressed = warmCompressor.process (driven);
        }
        else // Punch
        {
            compressed = punchCompressor.process (driven);
        }

        // Compensate Comp IN gain (decrease level after compression)
        compressed = compressed / compInGain;

        // Apply manual makeup gain
        float wet = compressed * makeupGain;

        // Mix dry and wet
        return dry * (1.0f - mixAmount) + wet * mixAmount;
    }

private:
    //==============================================================================
    void updateCompression()
    {
        // Fixed threshold at -10dB for both compressors
        if (currentAlgorithm == Warm)
        {
            warmCompressor.setParameters (-10.0f);  // Ratio 4:1 fixed internally
        }
        else // Punch
        {
            // Ratio 20:1, Attack 24ms, Release 10ms (very aggressive limiter-like)
            punchCompressor.setParameters (-10.0f, 20.0f, 24.0f, 10.0f);
        }
    }

    //==============================================================================
    CL1BCompressor warmCompressor;
    DigitalVersatileCompressor punchCompressor;

    Algorithm currentAlgorithm = Warm;  // Default: Warm

    // Comp IN (drive) parameters
    float compInDB = 0.0f;      // Default: 0dB (no drive)
    float compInGain = 1.0f;

    // Makeup gain parameters
    float makeupDB = 0.0f;      // Default: 0dB (no makeup)
    float makeupGain = 1.0f;

    // Mix parameters
    float mixPercent = 100.0f;  // Default: 100% (full wet)
    float mixAmount = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StyleCompSection)
};
