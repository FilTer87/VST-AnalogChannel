/*
  ==============================================================================

    DigitalVersatileCompressor.h
    Ported from Digital_Versatile_Compressor_V2.jsfx

    Original Author: Michael Gruhn (LOSER)
    Original Source: Docs/Plugins_source_code/Digital_Versatile_Compressor_V2.jsfx
    License: See original header (requires acknowledgment)

    Port to C++/JUCE: KuramaSound
    Date: 2024-11-06

    FAITHFUL PORT - Core DSP algorithm preserved exactly from original source.

    Clean, transparent compressor for peak control.
    Fixed parameters: Ratio 4:1, Peak detection (RMS=0), No auto-makeup

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>

//==============================================================================
/**
    Digital Versatile Compressor algorithm from JSFX.
    Faithful port with fixed ratio and two A/R presets.
*/
class DigitalVersatileCompressor
{
public:
    DigitalVersatileCompressor()
    {
        reset();
    }

    //==============================================================================
    void reset()
    {
        gain = 1.0f;
        seekGain = 1.0f;
        t = 0.0f;
        gr_meter = 1.0f;
    }

    void setSampleRate (double sampleRate)
    {
        currentSampleRate = sampleRate;

        // Peak detection filter coefficients
        b = static_cast<float>(-std::exp (-60.0 / currentSampleRate));
        a = static_cast<float>(1.0 + b);

        // GR meter decay (1 second)
        gr_meter_decay = static_cast<float>(std::exp (1.0 / (1.0 * currentSampleRate)));
    }

    //==============================================================================
    /**
        Set compression parameters.
        @param thresholdDB threshold in decibels (-30 to -0.1 dB)
        @param ratioValue compression ratio (e.g., 4.0 for 4:1, 7.0 for 7:1)
        @param attackMS attack time in milliseconds
        @param releaseMS release time in milliseconds
    */
    void setParameters (float thresholdDB, float ratioValue, float attackMS, float releaseMS)
    {
        threshDB = thresholdDB;
        thresh = std::exp (threshDB / c);

        // Ratio (e.g., 4.0 for 4:1, 7.0 for 7:1)
        ratio = 1.0f / ratioValue;

        // Attack/Release coefficients
        attack = static_cast<float>(std::exp (threshDB / (attackMS * currentSampleRate / 1000.0f) / c));
        release = static_cast<float>(std::exp (threshDB / (releaseMS * currentSampleRate / 1000.0f) / c));

        // No auto-makeup, no output gain
        volume = 1.0f;
    }

    //==============================================================================
    /**
        Process a single sample with compression.
        @param input the input sample
        @return the compressed sample
    */
    float process (float input)
    {
        // Peak detection with smooth filter
        // Original: rms = max(abs(spl0), abs(spl1)) for stereo
        // Mono: just abs(input)
        float rms = std::abs (input);
        rms = std::sqrt ((t = a * rms - b * t));

        // Compress mode only (not limit) - no additional max() operation
        // slider8=0 means compress mode, so we skip: rms = max(rms, rmsS)

        // RMS window disabled (rmsSize = 0, fixed for peak detection)

        // Gain computer
        seekGain = (rms > thresh)
                       ? std::exp ((threshDB + (std::log (rms) * c - threshDB) * ratio) / c) / rms
                       : 1.0f;

        // Smooth gain reduction (ballistics)
        gain = (gain > seekGain)
                   ? std::max (gain * attack, seekGain)   // Attack (gain going down)
                   : std::min (gain / release, seekGain); // Release (gain going up)

        // Apply compression
        float output = input * gain * volume;

        // Update gain reduction meter
        if (gain < gr_meter)
        {
            gr_meter = gain;
        }
        else
        {
            gr_meter *= gr_meter_decay;
            if (gr_meter > 1.0f)
                gr_meter = 1.0f;
        }

        return output;
    }

    //==============================================================================
    /**
        Get current gain reduction in decibels (for metering).
        @return gain reduction in dB (negative value, 0 = no reduction)
    */
    float getGainReductionDB() const
    {
        return (gr_meter > 0.0f) ? std::log (gr_meter) * (20.0f / std::log (10.0f)) : -150.0f;
    }

private:
    //==============================================================================
    double currentSampleRate = 44100.0;

    // State variables
    float gain = 1.0f;
    float seekGain = 1.0f;
    float t = 0.0f; // Peak detection filter state

    // Filter coefficients
    float b, a;

    // Gain reduction meter
    float gr_meter = 1.0f;
    float gr_meter_decay;

    // Parameters
    float threshDB;
    float thresh;
    float ratio;
    float attack;
    float release;
    float volume;

    // Constants
    const float c = 8.65617025f; // log conversion constant (20/ln(10))
    const float dc = 1e-30f;     // denormal prevention (not currently used)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DigitalVersatileCompressor)
};
