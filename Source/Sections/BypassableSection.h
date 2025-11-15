/*
  ==============================================================================

    BypassableSection.h
    Base class for all AnalogChannel processing sections
    Provides smooth bypass with 10ms crossfade

    Copyright (c) 2024 KuramaSound
    Licensed under GPL v3 - see LICENSE file for details

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Base class for all processing sections in AnalogChannel.
    Provides smooth bypass functionality with 10ms crossfade to avoid clicks/pops.

    Each section should inherit from this and implement processInternal().
*/
class BypassableSection
{
public:
    BypassableSection() = default;
    virtual ~BypassableSection() = default;

    //==============================================================================
    /**
        Sets the bypass state for this section.
        @param shouldBypass true to bypass, false to process
    */
    void setBypass (bool shouldBypass)
    {
        targetBypass = shouldBypass;
    }

    /**
        Returns the current bypass state.
    */
    bool isBypassed() const
    {
        return targetBypass;
    }

    /**
        Initializes the section with the current sample rate.
        Call this in prepareToPlay().
        @param sampleRate the sample rate in Hz
    */
    virtual void setSampleRate (double sampleRate)
    {
        currentSampleRate = sampleRate;
        // 10ms crossfade time
        fadeCoeff = 1.0f / (0.01f * static_cast<float>(sampleRate));
    }

    /**
        Resets the section's internal state.
        Call this when audio processing starts/stops.
    */
    virtual void reset()
    {
        // Derived classes can override to reset their state
    }

    /**
        Processes a single sample with smooth bypass crossfading.
        @param input the input sample
        @return the processed output sample
    */
    float process (float input)
    {
        // CRITICAL: If bypassed, return input immediately (no processing)
        if (targetBypass)
        {
            // Reset state immediately when bypass is activated
            if (bypassMix == 0.0f)
            {
                reset();  // Reset on first bypass call
            }

            // Smooth fade to bypass to avoid clicks
            if (bypassMix < 0.99f)
            {
                bypassMix += (1.0f - bypassMix) * fadeCoeff;
                if (bypassMix > 0.99f)
                {
                    bypassMix = 1.0f;
                }

                // During fade: NO PROCESSING, just crossfade dry signal
                // This prevents processing from affecting the signal during bypass
                return input;
            }
            else
            {
                // Fully bypassed: skip processing entirely (no processInternal call)
                return input;
            }
        }
        else
        {
            // Fade back to processing
            if (bypassMix > 0.01f)
            {
                bypassMix += (0.0f - bypassMix) * fadeCoeff;
                if (bypassMix < 0.01f) bypassMix = 0.0f;

                // During fade: mix wet and dry
                float wet = processInternal (input);
                return wet * (1.0f - bypassMix) + input * bypassMix;
            }
            else
            {
                // Fully active: process normally
                return processInternal (input);
            }
        }
    }

protected:
    /**
        Pure virtual function that derived classes must implement.
        This is where the actual DSP processing happens.
        @param input the input sample
        @return the processed output sample
    */
    virtual float processInternal (float input) = 0;

    double currentSampleRate = 44100.0;

private:
    bool targetBypass = false;      // Target bypass state
    float bypassMix = 0.0f;         // 0.0 = processing, 1.0 = bypassed
    float fadeCoeff = 0.0f;         // Crossfade coefficient

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassableSection)
};
