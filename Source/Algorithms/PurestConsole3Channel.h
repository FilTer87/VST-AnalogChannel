/*
  ==============================================================================

    PurestConsole3Channel.h
    Ported from AirWindows PurestConsole3Channel

    Original Author: Chris Johnson (Airwindows)
    Original Source: Docs/Plugins_source_code/PurestConsole3Channel/
    License: MIT

    Port to C++/JUCE: KuramaSound
    Date: 2024-11-06

    FAITHFUL PORT - Core DSP algorithm preserved exactly from original source.

    Purest console channel emulation - very subtle saturation.
    Polynomial waveshaping for transparent console character.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

//==============================================================================
/**
    PurestConsole3Channel algorithm from AirWindows.
    Very subtle console saturation with polynomial waveshaping.
*/
class PurestConsole3Channel
{
public:
    PurestConsole3Channel()
    {
        reset();
    }

    //==============================================================================
    void reset()
    {
        fpd = 17;
    }

    void setSampleRate (double sampleRate)
    {
        currentSampleRate = sampleRate;
    }

    //==============================================================================
    /**
        Process a single sample with console saturation.
        @param input the input sample
        @return the processed sample
    */
    float process (float input)
    {
        double inputSample = input;

        // Denormal prevention
        if (std::fabs (inputSample) < 1.18e-23)
            inputSample = fpd * 1.18e-17;

        // Polynomial waveshaping (crude sine approximation)
        // Original comment: "Note that because modern processors love math more than extra variables, this is optimized"
        inputSample += ((std::pow (inputSample, 5) / 128.0) + (std::pow (inputSample, 9) / 262144.0))
                     - ((std::pow (inputSample, 3) / 8.0) + (std::pow (inputSample, 7) / 4096.0));

        // PRNG for denormal prevention (chaotic noise generator)
        fpd ^= fpd << 13;
        fpd ^= fpd >> 17;
        fpd ^= fpd << 5;

        return static_cast<float> (inputSample);
    }

private:
    //==============================================================================
    double currentSampleRate = 44100.0;
    uint32_t fpd = 17;  // PRNG state for denormal prevention

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurestConsole3Channel)
};
