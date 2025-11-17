/*
  ==============================================================================

    Tube2.h
    Ported from AirWindows Tube2

    Original Author: Chris Johnson (Airwindows)
    Original Source: Docs/Plugins_source_code/Tube2/
    License: MIT

    Port to C++/JUCE: KuramaSound
    Date: 2024-11-06

    FAITHFUL PORT - Core DSP algorithm preserved exactly from original source.

    Tube saturation with asymmetric clipping and hysteresis.
    Three-stage algorithm:
    1. Asymmetric waveshaping (flatten bottom, point top)
    2. Powerfactor algorithm (tube-style saturation)
    3. Hysteresis and spiky fuzz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Tube2 saturation algorithm from AirWindows.
    Faithful port with drive control.
*/
class Tube2
{
public:
    Tube2()
    {
        reset();
    }

    //==============================================================================
    void setPRNGSeed (uint32_t /*seed*/)
    {
        // Tube2 doesn't use PRNG, this method exists for API consistency with ToTape8
        // No-op implementation
    }

    void reset()
    {
        previousSampleA = 0.0;
        previousSampleB = 0.0;
        previousSampleC = 0.0;
        previousSampleD = 0.0;
        previousSampleE = 0.0;
        previousSampleF = 0.0;
    }

    void setSampleRate (double sampleRate)
    {
        currentSampleRate = sampleRate;

        // Calculate overallscale for sample rate compensation
        overallscale = 1.0;
        overallscale /= 44100.0;
        overallscale *= currentSampleRate;
    }

    //==============================================================================
    /**
        Process a single sample with Tube2 saturation.
        @param input the input sample
        @param driveDB drive amount in decibels (-18 to +18 dB)
        @return the processed sample
    */
    float process (float input, float driveDB)
    {
        // Denormal prevention
        double inputSample = input;
        if (std::fabs (inputSample) < 1.18e-23)
            inputSample = 0.0;

        // New behavior: negative drive = volume only, positive drive = algorithm drive
        double A, B;
        if (driveDB < 0.0)
        {
            // Negative drive: just attenuate volume, keep algorithm at neutral (A=0.5, B=0.5)
            double volumeGain = std::pow (10.0, driveDB / 20.0);
            inputSample *= volumeGain;
            A = 0.5;  // Algorithm stays neutral
            B = 0.5;
        }
        else
        {
            // Positive drive: map 0..+18 dB to A/B 0.5..1.0
            A = 0.5 + (driveDB / 36.0);  // 0 dB → 0.5, +18 dB → 1.0
            A = juce::jlimit (0.5, 1.0, A);
            B = A;  // Parameter B follows A
        }

        double inputPad = A;  // Parameter A in original
        double iterations = 1.0 - B;  // Original uses 1.0-B

        int powerfactor = static_cast<int> ((9.0 * iterations) + 1.0);
        double asymPad = static_cast<double> (powerfactor);
        double gainscaling = 1.0 / static_cast<double> (powerfactor + 1);
        double outputscaling = 1.0 + (1.0 / static_cast<double> (powerfactor));

        // Input attenuation
        if (inputPad < 1.0)
            inputSample *= inputPad;

        // For high sample rates, do simple averaging
        if (overallscale > 1.9)
        {
            double stored = inputSample;
            inputSample += previousSampleA;
            previousSampleA = stored;
            inputSample *= 0.5;
        }

        // Hard clip to ±1.0
        if (inputSample > 1.0) inputSample = 1.0;
        if (inputSample < -1.0) inputSample = -1.0;

        // Flatten bottom, point top of sine waveshaper
        inputSample /= asymPad;
        double sharpen = -inputSample;
        if (sharpen > 0.0)
            sharpen = 1.0 + std::sqrt (sharpen);
        else
            sharpen = 1.0 - std::sqrt (-sharpen);

        inputSample -= inputSample * std::fabs (inputSample) * sharpen * 0.25;
        inputSample *= asymPad;

        // Original Tube algorithm: powerfactor widens the more linear region
        double factor = inputSample;
        for (int x = 0; x < powerfactor; x++)
            factor *= inputSample;

        if ((powerfactor % 2 == 1) && (inputSample != 0.0))
            factor = (factor / inputSample) * std::fabs (inputSample);

        factor *= gainscaling;
        inputSample -= factor;
        inputSample *= outputscaling;

        // For high sample rates, averaging again
        if (overallscale > 1.9)
        {
            double stored = inputSample;
            inputSample += previousSampleC;
            previousSampleC = stored;
            inputSample *= 0.5;
        }

        // Hysteresis and spiky fuzz
        double slew = previousSampleE - inputSample;

        if (overallscale > 1.9)
        {
            double stored = inputSample;
            inputSample += previousSampleE;
            previousSampleE = stored;
            inputSample *= 0.5;
        }
        else
        {
            previousSampleE = inputSample;
        }

        if (slew > 0.0)
            slew = 1.0 + (std::sqrt (slew) * 0.5);
        else
            slew = 1.0 - (std::sqrt (-slew) * 0.5);

        inputSample -= inputSample * std::fabs (inputSample) * slew * gainscaling;

        // Hard clip
        if (inputSample > 0.52) inputSample = 0.52;
        if (inputSample < -0.52) inputSample = -0.52;

        inputSample *= 1.923076923076923;

        return static_cast<float> (inputSample);
    }

private:
    //==============================================================================
    double currentSampleRate = 44100.0;
    double overallscale = 1.0;

    // State variables for averaging/hysteresis
    double previousSampleA = 0.0;
    double previousSampleB = 0.0; // Not used in mono
    double previousSampleC = 0.0;
    double previousSampleD = 0.0; // Not used in mono
    double previousSampleE = 0.0;
    double previousSampleF = 0.0; // Not used in mono

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tube2)
};
