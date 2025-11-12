/*
  ==============================================================================

    ClipSoftly.h
    Ported from AirWindows ClipSoftly

    Original Author: Chris Johnson (Airwindows)
    Original Source: Docs/Plugins_source_code/ClipSoftly/
    License: MIT

    Port to C++/JUCE: KuramaSound
    Date: 2024-11-06

    FAITHFUL PORT - Core DSP algorithm preserved exactly from original source.

    Soft clipper with sin waveshaping and adaptive smoothing.
    Uses dynamic "softSpeed" to smooth the clipping based on signal level.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

//==============================================================================
/**
    ClipSoftly algorithm from AirWindows.
    Soft clipper with sin waveshaping and adaptive smoothing.
*/
class ClipSoftly
{
public:
    ClipSoftly()
    {
        reset();
    }

    //==============================================================================
    void reset()
    {
        lastSample = 0.0;
        fpd = 17;

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
        Process a single sample with soft clipping.
        @param input the input sample
        @return the processed sample
    */
    float process (float input)
    {
        double inputSample = input;

        // Denormal prevention
        if (std::fabs (inputSample) < 1.18e-23)
            inputSample = fpd * 1.18e-17;

        // Calculate adaptive smoothing factor
        double softSpeed = std::fabs (inputSample);
        if (softSpeed < 1.0)
            softSpeed = 1.0;
        else
            softSpeed = 1.0 / softSpeed;

        // Clip to ±π/2
        if (inputSample > 1.57079633) inputSample = 1.57079633;
        if (inputSample < -1.57079633) inputSample = -1.57079633;

        // Sin waveshaping, scaled to match ClipOnly (-0.2dB)
        inputSample = std::sin (inputSample) * 0.9549925859;

        // Adaptive smoothing blend with last sample
        inputSample = (inputSample * softSpeed) + (lastSample * (1.0 - softSpeed));

        // Latency compensation buffer
        intermediate[spacing] = inputSample;
        inputSample = lastSample; // Latency is however many samples equals one 44.1k sample

        for (int x = spacing; x > 0; x--)
            intermediate[x - 1] = intermediate[x];

        lastSample = intermediate[0]; // Run a little buffer to handle this

        // PRNG for denormal prevention
        fpd ^= fpd << 13;
        fpd ^= fpd >> 17;
        fpd ^= fpd << 5;

        return static_cast<float> (inputSample);
    }

private:
    //==============================================================================
    double currentSampleRate = 44100.0;
    int spacing = 1;

    // State variables
    double lastSample = 0.0;
    double intermediate[16];
    uint32_t fpd = 17; // PRNG state for denormal prevention

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipSoftly)
};
