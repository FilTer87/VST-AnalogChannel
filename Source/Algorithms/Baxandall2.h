/*
  ==============================================================================

    Baxandall2.h
    Ported from AirWindows Baxandall2

    Original Author: Chris Johnson (Airwindows)
    Original Source: Docs/Plugins_source_code/Baxandall2/
    License: MIT

    Port to C++/JUCE: KuramaSound
    Date: 2024-11-06

    FAITHFUL PORT - Core DSP algorithm preserved exactly from original source.

    Baxandall-style shelving EQ with separate bass and treble controls.
    Uses biquad filters with interleaved processing and "flip" technique
    for improved numerical stability.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Baxandall2 shelving EQ from AirWindows.
    Independent bass and treble shelf controls.
*/
class Baxandall2
{
public:
    Baxandall2()
    {
        reset();
    }

    //==============================================================================
    void reset()
    {
        // Initialize all filter state arrays to zero
        for (int i = 0; i < 9; ++i)
        {
            trebleA[i] = 0.0;
            trebleB[i] = 0.0;
            bassA[i] = 0.0;
            bassB[i] = 0.0;
        }
        flip = false;
    }

    void setSampleRate (double sampleRate)
    {
        currentSampleRate = sampleRate;
        updateCoefficients();
    }

    //==============================================================================
    /**
        Set bass shelf gain.
        @param dB gain from -24 to +24 dB (we'll use -15 to +15 in AnalogChannel)
    */
    void setBass (float dB)
    {
        bassGainDB = dB;
        updateCoefficients();
    }

    /**
        Set treble shelf gain.
        @param dB gain from -24 to +24 dB (we'll use -15 to +15 in AnalogChannel)
    */
    void setTreble (float dB)
    {
        trebleGainDB = dB;
        updateCoefficients();
    }

    /**
        Set treble shelf frequency.
        @param hz frequency in Hz (default: 4410 Hz for normalized 0.1 at 44.1kHz)
    */
    void setTrebleFreq (float hz)
    {
        trebleFreqHz = hz;
        updateCoefficients();
    }

    /**
        Set bass shelf frequency.
        @param hz frequency in Hz (default: 8820 Hz for normalized 0.2 at 44.1kHz)
    */
    void setBassFreq (float hz)
    {
        bassFreqHz = hz;
        updateCoefficients();
    }

    /**
        Process a single sample.
    */
    float process (float input)
    {
        // Denormal prevention (from original)
        double inputSample = input;
        if (std::fabs (inputSample) < 1.18e-23)
            inputSample = 0.0;

        double trebleSample, bassSample;

        // Interleaved biquad processing with flip for numerical stability
        if (flip)
        {
            // Treble shelf (highpass-derived)
            trebleSample = (inputSample * trebleA[2]) + trebleA[7];
            trebleA[7] = (inputSample * trebleA[3]) - (trebleSample * trebleA[5]) + trebleA[8];
            trebleA[8] = (inputSample * trebleA[4]) - (trebleSample * trebleA[6]);
            trebleSample = inputSample - trebleSample;  // Highpass component

            // Bass shelf (lowpass-derived)
            bassSample = (inputSample * bassA[2]) + bassA[7];
            bassA[7] = (inputSample * bassA[3]) - (bassSample * bassA[5]) + bassA[8];
            bassA[8] = (inputSample * bassA[4]) - (bassSample * bassA[6]);
        }
        else
        {
            // Treble shelf (highpass-derived)
            trebleSample = (inputSample * trebleB[2]) + trebleB[7];
            trebleB[7] = (inputSample * trebleB[3]) - (trebleSample * trebleB[5]) + trebleB[8];
            trebleB[8] = (inputSample * trebleB[4]) - (trebleSample * trebleB[6]);
            trebleSample = inputSample - trebleSample;  // Highpass component

            // Bass shelf (lowpass-derived)
            bassSample = (inputSample * bassB[2]) + bassB[7];
            bassB[7] = (inputSample * bassB[3]) - (bassSample * bassB[5]) + bassB[8];
            bassB[8] = (inputSample * bassB[4]) - (bassSample * bassB[6]);
        }

        flip = !flip;

        // Apply gains and combine
        trebleSample *= trebleGainLinear;
        bassSample *= bassGainLinear;

        // Interleaved biquad output
        double output = bassSample + trebleSample;

        return static_cast<float> (output);
    }

private:
    //==============================================================================
    void updateCoefficients()
    {
        if (currentSampleRate <= 0.0)
            return;

        // Calculate treble gain (linear)
        trebleGainLinear = std::pow (10.0, trebleGainDB / 20.0);

        // Treble frequency (adaptive based on gain, using user-selectable base frequency)
        double trebleFreq = (trebleFreqHz * trebleGainLinear) / currentSampleRate;
        if (trebleFreq > 0.45)
            trebleFreq = 0.45;

        // Calculate bass gain (linear)
        bassGainLinear = std::pow (10.0, bassGainDB / 20.0);

        // Bass frequency (inverse relationship with gain, using user-selectable base frequency)
        double bassFreq = std::pow (10.0, -bassGainDB / 20.0);
        bassFreq = (bassFreqHz * bassFreq) / currentSampleRate;
        if (bassFreq > 0.45)
            bassFreq = 0.45;

        // Set filter parameters
        trebleA[0] = trebleB[0] = trebleFreq;
        bassA[0] = bassB[0] = bassFreq;

        trebleA[1] = trebleB[1] = 0.4;  // Q for treble
        bassA[1] = bassB[1] = 0.2;      // Q for bass

        // Calculate biquad coefficients for TREBLE
        double K = std::tan (juce::MathConstants<double>::pi * trebleA[0]);
        double norm = 1.0 / (1.0 + K / trebleA[1] + K * K);

        trebleA[2] = trebleB[2] = K * K * norm;
        trebleA[3] = trebleB[3] = 2.0 * trebleA[2];
        trebleA[4] = trebleB[4] = trebleA[2];
        trebleA[5] = trebleB[5] = 2.0 * (K * K - 1.0) * norm;
        trebleA[6] = trebleB[6] = (1.0 - K / trebleA[1] + K * K) * norm;

        // Calculate biquad coefficients for BASS
        K = std::tan (juce::MathConstants<double>::pi * bassA[0]);
        norm = 1.0 / (1.0 + K / bassA[1] + K * K);

        bassA[2] = bassB[2] = K * K * norm;
        bassA[3] = bassB[3] = 2.0 * bassA[2];
        bassA[4] = bassB[4] = bassA[2];
        bassA[5] = bassB[5] = 2.0 * (K * K - 1.0) * norm;
        bassA[6] = bassB[6] = (1.0 - K / bassA[1] + K * K) * norm;
    }

    //==============================================================================
    double currentSampleRate = 44100.0;
    float bassGainDB = 0.0f;
    float trebleGainDB = 0.0f;
    float bassFreqHz = 8820.0f;    // Default: matches original Baxandall2 behavior
    float trebleFreqHz = 4410.0f;  // Default: matches original Baxandall2 behavior
    double bassGainLinear = 1.0;
    double trebleGainLinear = 1.0;

    // Biquad state arrays (indices 0-6 = coefficients, 7-8 = state)
    double trebleA[9] = {0.0};
    double trebleB[9] = {0.0};
    double bassA[9] = {0.0};
    double bassB[9] = {0.0};

    bool flip = false;  // For interleaved processing

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Baxandall2)
};
