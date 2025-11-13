/*
  ==============================================================================

    ControlCompSection.h
    Section 3: Control-Comp (Clean Compressor for Peak Control)
    Algorithm: Digital Versatile Compressor

  ==============================================================================
*/

#pragma once

#include "BypassableSection.h"
#include "../Algorithms/DigitalVersatileCompressor.h"

//==============================================================================
/**
    Control-Comp section - Clean, transparent compression for peak control.
    Fixed ratio 4:1, two A/R presets (Fast/Normal).
*/
class ControlCompSection : public BypassableSection
{
public:
    enum ARMode
    {
        Fast = 0,   // Attack=0.2ms, Release=40ms, Ratio=4:1
        Normal = 1  // Attack=30ms, Release=100ms, Ratio=2.5:1
    };

    ControlCompSection() = default;

    //==============================================================================
    void setSampleRate (double sampleRate) override
    {
        BypassableSection::setSampleRate (sampleRate);
        compressor.setSampleRate (sampleRate);
        updateCompressorParameters();
    }

    void reset() override
    {
        compressor.reset();
    }

    //==============================================================================
    /**
        Sets the compression threshold.
        @param dB threshold from -30 dB to -0.1 dB
    */
    void setThreshold (float dB)
    {
        thresholdDB = juce::jlimit (-30.0f, -0.1f, dB);
        updateCompressorParameters();
    }

    /**
        Sets the attack/release mode.
        @param mode Fast or Normal preset
    */
    void setARMode (ARMode mode)
    {
        arMode = mode;
        updateCompressorParameters();
    }

    /**
        Get current gain reduction for metering.
        @return gain reduction in dB (negative value)
    */
    float getGainReductionDB() const
    {
        return compressor.getGainReductionDB();
    }

protected:
    //==============================================================================
    float processInternal (float input) override
    {
        return compressor.process (input);
    }

private:
    //==============================================================================
    void updateCompressorParameters()
    {
        // Attack/Release presets
        // Both modes: RMS Size=0 (peak), Auto Make-up=NO, Output=0dB, Character=Compress (NO limit)
        float attackMS, releaseMS, ratioValue;
        if (arMode == Fast)
        {
            attackMS = 0.2f;
            releaseMS = 40.0f;
            ratioValue = 4.0f;  // Fast mode: Ratio 4:1
        }
        else // Normal
        {
            attackMS = 30.0f;
            releaseMS = 100.0f;
            ratioValue = 2.5f;  // Normal mode: Ratio 2.5:1
        }

        compressor.setParameters (thresholdDB, ratioValue, attackMS, releaseMS);
    }

    //==============================================================================
    DigitalVersatileCompressor compressor;

    float thresholdDB = -10.0f;  // Default threshold
    ARMode arMode = Normal;       // Default: Normal A/R

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlCompSection)
};
