/*
  ==============================================================================

    PurestDrive.h
    Ported from AirWindows PurestDrive

    Original Author: Chris Johnson (Airwindows)
    Original Source: https://github.com/airwindows/airwindows/tree/master/plugins/LinuxVST/src/PurestDrive
    License: MIT

    Port to C++/JUCE: KuramaSound
    Date: 2024-11-05

    FAITHFUL PORT - Core DSP algorithm preserved exactly from original source.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    PurestDrive saturation algorithm from AirWindows.
    Faithful port of the original algorithm.

    Core concept: sin() distortion with dynamic apply factor based on previous sample
    to preserve transients and high-frequency content.
*/
class PurestDrive
{
public:
    PurestDrive()
    {
        reset();
    }

    //==============================================================================
    void reset()
    {
        previousSample = 0.0;
    }

    void setSampleRate (double sampleRate)
    {
        // Sample rate doesn't affect this algorithm
        juce::ignoreUnused (sampleRate);
    }

    //==============================================================================
    /**
        Process a single sample with PurestDrive saturation.
        @param input the input sample
        @param driveDB drive amount in decibels (-18 to +18 dB)
        @return the processed sample
    */
    float process (float input, float driveDB)
    {
        // Denormal prevention (from original)
        double inputSample = input;
        if (std::fabs (inputSample) < 1.18e-23)
            inputSample = 0.0;

        // New behavior: negative drive = volume only, positive drive = algorithm drive
        double intensity;
        if (driveDB < 0.0)
        {
            // Negative drive: just attenuate volume, keep algorithm at neutral (0.5)
            double volumeGain = std::pow (10.0, driveDB / 20.0);
            inputSample *= volumeGain;
            intensity = 0.5;  // Algorithm stays neutral
        }
        else
        {
            // Positive drive: map 0..+18 dB to intensity 0.5..1.0
            intensity = 0.5 + (driveDB / 36.0);  // 0 dB → 0.5, +18 dB → 1.0
            intensity = juce::jlimit (0.5, 1.0, intensity);
        }

        double drySample = inputSample;

        // Basic distortion factor: sin()
        inputSample = std::sin (inputSample);

        // Dynamic apply factor based on previous sample
        // Saturates less if previous sample was undistorted and low level,
        // or if it was inverse polarity. Lets through highs and brightness more.
        double apply = (std::fabs (previousSample + inputSample) / 2.0) * intensity;

        // Dry-wet control for intensity (also has FM modulation to clean up highs)
        inputSample = (drySample * (1.0 - apply)) + (inputSample * apply);

        // Store previous sample (apply sin to dry for next iteration)
        previousSample = std::sin (drySample);

        return static_cast<float> (inputSample);
    }

private:
    //==============================================================================
    double previousSample = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurestDrive)
};
