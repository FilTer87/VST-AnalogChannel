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
        // If fully bypassed and crossfade is complete, skip processing entirely
        if (targetBypass && bypassMix >= 0.9999f)
        {
            return input;
        }

        // Process wet signal
        float wet = processInternal (input);

        // Smooth crossfade between wet and dry
        float targetMix = targetBypass ? 1.0f : 0.0f;

        if (bypassMix != targetMix)
        {
            bypassMix += (targetMix - bypassMix) * fadeCoeff;

            // Snap to target when very close to avoid denormals
            if (juce::approximatelyEqual (bypassMix, targetMix))
                bypassMix = targetMix;
        }

        // Mix wet and dry signals
        // bypassMix = 0.0: fully wet (processing)
        // bypassMix = 1.0: fully dry (bypassed)
        return wet * (1.0f - bypassMix) + input * bypassMix;
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
