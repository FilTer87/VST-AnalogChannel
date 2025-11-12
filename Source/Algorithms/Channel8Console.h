/*
  ==============================================================================

    Channel8Console.h
    Professional console emulation algorithm from Airwindows Channel8

    Original code: Copyright (c) 2016 airwindows
    Airwindows uses the MIT license
    
    Ported and adapted for AnalogChannel by: KuramaSound

    Emulates three classic analog console types:
    - Neve: Warm, smooth, rounded transients (highest HPF, lowest slew threshold)
    - API: Punchy, preserves low end (lowest HPF, medium slew threshold)
    - SSL: Tight, clean, fast transients (medium HPF, highest slew threshold)

    Four-stage processing:
    1. Adaptive highpass filter with dielectric absorption modeling
    2. Dual saturation system (Spiral + Phat)
    3. Golden ratio slew rate limiter
    4. TPDF dithering

    Drive and Output parameters are fixed internally:
    - drive = 0.5 (100%, center position)
    - output = 0.83 (gain matching)

    External drive control is applied pre/post via ConsoleSection.

  ==============================================================================
*/

#pragma once

#include <cmath>
#include <cstdint>

//==============================================================================
/**
    Channel8Console - Professional console emulation from Airwindows.
    Single-channel processor (use separate instances for stereo).
*/
class Channel8Console
{
public:
    enum ConsoleType
    {
        Neve = 0,  // Essex replacement: warm, smooth
        API = 1,   // USA replacement: punchy, low-end
        SSL = 2    // Oxford replacement: tight, clean
    };

    //==============================================================================
    Channel8Console()
    {
        reset();
        setSampleRate (44100.0);
        setConsoleType (SSL);  // Default to SSL
    }

    //==============================================================================
    /**
        Sets the console type (Neve, API, or SSL).
        Changes the highpass filter frequency and slew rate threshold.
    */
    void setConsoleType (ConsoleType type)
    {
        switch (type)
        {
            case Neve:
                iirAmount = 0.005832;     // Highest HPF frequency
                threshold = 0.33362176;   // Lowest slew threshold (most rounding)
                break;

            case API:
                iirAmount = 0.004096;     // Lowest HPF frequency
                threshold = 0.59969536;   // Medium slew threshold
                break;

            case SSL:
                iirAmount = 0.004913;     // Medium HPF frequency
                threshold = 0.84934656;   // Highest slew threshold (least rounding)
                break;
        }

        currentType = type;
    }

    /**
        Sets the sample rate and updates internal scaling.
    */
    void setSampleRate (double sampleRate)
    {
        currentSampleRate = sampleRate;

        // Scale filter coefficient for sample rate
        double overallscale = sampleRate / 44100.0;
        localIirAmount = iirAmount / overallscale;
    }

    /**
        Resets all internal state variables.
    */
    void reset()
    {
        iirSampleA = 0.0;
        iirSampleB = 0.0;
        lastSampleA = 0.0;
        lastSampleB = 0.0;
        lastSampleC = 0.0;
        flip = false;
        fpd = 1;
    }

    /**
        Processes a single audio sample through the console emulation.
        @param input Input sample (-1.0 to +1.0 range)
        @return Processed output sample
    */
    float process (float input)
    {
        double inputSample = static_cast<double> (input);

        // Denormal prevention
        if (std::fabs (inputSample) < 1.18e-23)
            inputSample = fpd * 1.18e-17;

        // Stage 1: Adaptive Highpass Filter (Dielectric Absorption)
        // ============================================================
        // The filter frequency changes based on signal amplitude
        double dielectricScale = std::fabs (2.0 - ((inputSample + nonLin) / nonLin));

        if (flip)
        {
            iirSampleA = (iirSampleA * (1.0 - (localIirAmount * dielectricScale))) +
                         (inputSample * localIirAmount * dielectricScale);
            inputSample = inputSample - iirSampleA;
        }
        else
        {
            iirSampleB = (iirSampleB * (1.0 - (localIirAmount * dielectricScale))) +
                         (inputSample * localIirAmount * dielectricScale);
            inputSample = inputSample - iirSampleB;
        }

        // Stage 2: Dual Saturation System
        // ============================================================
        double drySample = inputSample;

        // Hard clip to prevent runaway
        if (inputSample > 1.0) inputSample = 1.0;
        if (inputSample < -1.0) inputSample = -1.0;

        // Phat saturation - simple sine waveshaping (warmer)
        double phatSample = std::sin (inputSample * 1.57079633);

        // Prepare for Spiral saturation
        inputSample *= 1.2533141373155;

        // Spiral saturation - complex formula (asymmetric harmonics)
        double distSample = std::sin (inputSample * std::fabs (inputSample)) /
                           ((std::fabs (inputSample) == 0.0) ? 1.0 : std::fabs (inputSample));

        // Blend saturation types based on drive
        inputSample = distSample;  // Start with full Spiral

        if (density < 1.0)
            inputSample = (drySample * (1.0 - density)) + (distSample * density);  // Fade in Spiral

        if (phattity > 0.0)
            inputSample = (inputSample * (1.0 - phattity)) + (phatSample * phattity);  // Add Phat on top

        // Stage 3: Golden Ratio Slew Rate Limiter
        // ============================================================
        // Calculate weighted slew rate using golden ratio constants
        double clamp = (lastSampleB - lastSampleC) * 0.381966011250105;
        clamp -= (lastSampleA - lastSampleB) * 0.6180339887498948482045;
        clamp += inputSample - lastSampleA;

        // Shift history
        lastSampleC = lastSampleB;
        lastSampleB = lastSampleA;
        lastSampleA = inputSample;

        // Apply threshold limiting
        if (clamp > threshold)
            inputSample = lastSampleB + threshold;
        if (-clamp > threshold)
            inputSample = lastSampleB - threshold;

        // Blend limited with raw using golden ratio
        lastSampleA = (lastSampleA * 0.381966011250105) + (inputSample * 0.6180339887498948482045);

        // Alternate filter banks
        flip = !flip;

        // Output gain (fixed at 0.83 for gain matching)
        if (output < 1.0)
            inputSample *= output;

        // Stage 4: TPDF Dithering
        // ============================================================
        int expon;
        std::frexp (static_cast<float> (inputSample), &expon);
        fpd ^= fpd << 13;
        fpd ^= fpd >> 17;
        fpd ^= fpd << 5;
        inputSample += ((static_cast<double> (fpd) - static_cast<uint32_t> (0x7fffffff)) *
                       5.5e-36 * std::pow (2.0, expon + 62));

        return static_cast<float> (inputSample);
    }

private:
    //==============================================================================
    // Console-specific parameters (set by setConsoleType)
    double iirAmount = 0.004913;      // Highpass filter coefficient
    double threshold = 0.84934656;    // Slew rate threshold
    ConsoleType currentType = SSL;

    // Fixed internal parameters (drive at 100%, output at 0.83)
    static constexpr double drive = 0.5;     // 100% (center position)
    static constexpr double output = 0.83;   // Gain matching

    // Derived drive parameters
    static constexpr double density = drive * 2.0;  // 1.0 at 100% drive
    static constexpr double phattity = density - 1.0;  // 0.0 at 100% drive
    static constexpr double nonLin = 5.0 - density;  // 4.0 at 100% drive

    // Sample rate dependent
    double currentSampleRate = 44100.0;
    double localIirAmount = 0.004913;

    // State variables
    double iirSampleA = 0.0;
    double iirSampleB = 0.0;
    double lastSampleA = 0.0;
    double lastSampleB = 0.0;
    double lastSampleC = 0.0;
    bool flip = false;
    uint32_t fpd = 1;  // Pseudo-random number for dithering

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Channel8Console)
};
