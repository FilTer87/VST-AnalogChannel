/*
  ==============================================================================

    FinalClip.h
    Ported from AirWindows FinalClip

    Original Author: Chris Johnson (Airwindows)
    Original Source: Docs/Plugins_source_code/FinalClip/
    License: MIT

    Port to C++/JUCE: KuramaSound
    Date: 2024-11-06

    FAITHFUL PORT - Core DSP algorithm preserved exactly from original source.

    Hard clipper with slew limiting and golden ratio soft-knee.
    Uses ClipOnly2 algorithm with latency compensation buffer.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

//==============================================================================
/**
    FinalClip algorithm from AirWindows.
    Hard clipper with golden ratio soft-knee and slew limiting.
*/
class FinalClip
{
public:
    FinalClip()
    {
        reset();
    }

    //==============================================================================
    void reset()
    {
        lastSample = 0.0;
        wasPosClip = false;
        wasNegClip = false;

        for (int i = 0; i < 16; ++i)
            intermediate[i] = 0.0;
    }

    void setSampleRate (double sampleRate)
    {
        currentSampleRate = sampleRate;

        // Calculate overallscale for sample rate compensation
        double overallscale = currentSampleRate / 44100.0;

        // Spacing for latency compensation buffer (usually 2 or 4)
        spacing = static_cast<int> (std::floor (overallscale));
        if (spacing < 1) spacing = 1;
        if (spacing > 16) spacing = 16;
    }

    //==============================================================================
    /**
        Process a single sample with hard clipping.
        @param input the input sample
        @return the processed sample
    */
    float process (float input)
    {
        double inputSample = input;

        // Hard clip to ±4.0
        if (inputSample > 4.0) inputSample = 4.0;
        if (inputSample < -4.0) inputSample = -4.0;

        // Slew limiting (golden ratio)
        if (inputSample - lastSample > 0.618033988749894)
            inputSample = lastSample + 0.618033988749894;
        if (inputSample - lastSample < -0.618033988749894)
            inputSample = lastSample - 0.618033988749894;

        // ===== ClipOnly2 algorithm =====
        // Positive clip handling
        if (wasPosClip == true)
        {
            // Current will be over
            if (inputSample < lastSample)
                lastSample = 1.0 + (inputSample * 0.381966011250105);
            else
                lastSample = 0.618033988749894 + (lastSample * 0.618033988749894);
        }
        wasPosClip = false;

        if (inputSample > 1.618033988749894)
        {
            wasPosClip = true;
            inputSample = 1.0 + (lastSample * 0.381966011250105);
        }

        // Negative clip handling
        if (wasNegClip == true)
        {
            // Current will be -over
            if (inputSample > lastSample)
                lastSample = -1.0 + (inputSample * 0.381966011250105);
            else
                lastSample = -0.618033988749894 + (lastSample * 0.618033988749894);
        }
        wasNegClip = false;

        if (inputSample < -1.618033988749894)
        {
            wasNegClip = true;
            inputSample = -1.0 + (lastSample * 0.381966011250105);
        }

        // Latency compensation buffer
        intermediate[spacing] = inputSample;
        inputSample = lastSample; // Latency is however many samples equals one 44.1k sample

        for (int x = spacing; x > 0; x--)
            intermediate[x - 1] = intermediate[x];

        lastSample = intermediate[0]; // Run a little buffer to handle this

        return static_cast<float> (inputSample);
    }

private:
    //==============================================================================
    double currentSampleRate = 44100.0;
    int spacing = 1;

    // ClipOnly2 state
    double lastSample = 0.0;
    double intermediate[16];
    bool wasPosClip = false;
    bool wasNegClip = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FinalClip)
};
