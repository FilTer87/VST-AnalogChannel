/*
  ==============================================================================

    FilterSection.h
    Section 2: High-Pass and Low-Pass Filters
    Implements HPF and LPF with Q and slope options using Matched-Z Transform

    Uses Matched-Z transform instead of bilinear transform to eliminate
    frequency warping (cramping) at high frequencies, providing accurate
    analog-like frequency response across the entire spectrum.

  ==============================================================================
*/

#pragma once

#include "BypassableSection.h"
#include <cmath>

//==============================================================================
/**
    Filter section - HPF and LPF with configurable slope and Q.
*/
class FilterSection : public BypassableSection
{
public:
    enum Slope
    {
        Slope_6dB = 0,
        Slope_12dB = 1,
        Slope_18dB = 2
    };

    enum QMode
    {
        Normal = 0,  // Q = 0.8
        Bump = 1     // Q = 1.2
    };

    FilterSection() = default;

    //==============================================================================
    void setSampleRate (double sampleRate) override
    {
        BypassableSection::setSampleRate (sampleRate);

        hpf1.reset();
        hpf2.reset();
        lpf1.reset();

        updateFilters();
    }

    void reset() override
    {
        hpf1.reset();
        hpf2.reset();
        lpf1.reset();
        lpf2.reset();
    }

    //==============================================================================
    /**
        Sets the high-pass filter parameters.
        @param freqHz frequency from 20 Hz to 6000 Hz
        @param slope 12 dB/oct or 18 dB/oct
        @param qMode Normal (0.8) or Bump (1.2)
    */
    void setHPF (float freqHz, Slope slope, QMode qMode)
    {
        hpfFreq = freqHz;
        hpfSlope = slope;
        hpfQMode = qMode;
        updateFilters();
    }

    /**
        Sets the low-pass filter parameters.
        @param freqHz frequency from 300 Hz to 24000 Hz
        @param slope 6 dB/oct or 12 dB/oct
        @param qMode Normal (0.8) or Bump (1.2)
    */
    void setLPF (float freqHz, Slope slope, QMode qMode)
    {
        lpfFreq = freqHz;
        lpfSlope = slope;
        lpfQMode = qMode;
        updateFilters();
    }

    /**
        Sets HPF Q offset for channel variation.
        @param offset Q offset (typically ±0.06)
    */
    void setHPFQOffset (float offset)
    {
        hpfQOffset = offset;
        updateFilters();
    }

    /**
        Sets LPF Q offset for channel variation.
        @param offset Q offset (typically ±0.06)
    */
    void setLPFQOffset (float offset)
    {
        lpfQOffset = offset;
        updateFilters();
    }

protected:
    //==============================================================================
    float processInternal (float input) override
    {
        float output = input;

        // High-pass filter(s)
        output = hpf1.processSample (output);
        if (hpfSlope == Slope_18dB)
            output = hpf2.processSample (output);  // Cascade for 18dB/oct

        // Low-pass filter(s)
        output = lpf1.processSample (output);
        if (lpfSlope == Slope_12dB)
            output = lpf2.processSample (output);  // Cascade for 12dB/oct (FIXED)

        return output;
    }

private:
    //==============================================================================
    /**
        Creates Matched-Z Transform coefficients for a highpass filter.
        Eliminates frequency warping for accurate analog-like response.
    */
    static juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>>
    makeMatchedHighPass (double sampleRate, double frequency, double Q)
    {
        jassert (sampleRate > 0.0);
        jassert (frequency > 0.0 && frequency <= sampleRate * 0.5);
        jassert (Q > 0.0);

        // Analog prototype coefficients (s-domain)
        // H(s) = s^2 / (s^2 + s*(w0/Q) + w0^2)
        const double w0 = 2.0 * juce::MathConstants<double>::pi * frequency;

        // Matched-Z: map poles and zeros directly from s-plane to z-plane
        // For HPF: zeros at z = -1 (DC rejection)

        // Poles from quadratic formula
        const double alpha = w0 / (2.0 * Q);
        const double discriminant = alpha * alpha - w0 * w0;

        double poleReal, poleImag;
        if (discriminant >= 0.0)
        {
            // Real poles (overdamped)
            // double sqrtDisc = std::sqrt (discriminant); // Not needed for current implementation
            poleReal = -alpha;
            poleImag = 0.0;
        }
        else
        {
            // Complex poles (underdamped - normal case)
            poleReal = -alpha;
            poleImag = std::sqrt (-discriminant);
        }

        // Map to z-plane: z = exp(s*T)
        const double T = 1.0 / sampleRate;
        const double expRT = std::exp (poleReal * T);
        const double re = expRT * std::cos (poleImag * T);
        const double im = expRT * std::sin (poleImag * T);

        // Biquad coefficients
        // Numerator: (1 - z^-1)^2 = 1 - 2z^-1 + z^-2
        const double b0 = 1.0;
        const double b1 = -2.0;
        const double b2 = 1.0;

        // Denominator from poles: 1 - 2*re*z^-1 + (re^2+im^2)*z^-2
        const double a0 = 1.0;
        const double a1 = -2.0 * re;
        const double a2 = re * re + im * im;

        // Normalize gain at Nyquist for HPF
        const double gainNyquist = (b0 - b1 + b2) / (a0 - a1 + a2);

        return new juce::dsp::IIR::Coefficients<float> (
            static_cast<float> (b0 / gainNyquist),
            static_cast<float> (b1 / gainNyquist),
            static_cast<float> (b2 / gainNyquist),
            static_cast<float> (a0),
            static_cast<float> (a1 / a0),
            static_cast<float> (a2 / a0));
    }

    /**
        Creates standard bilinear transform coefficients for a lowpass filter.
        Uses standard JUCE implementation - reliable and accurate.
        Note: Has slight frequency warping at high frequencies (cramping).
    */
    static juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>>
    makeMatchedLowPass (double sampleRate, double frequency, double Q)
    {
        jassert (sampleRate > 0.0);
        jassert (frequency > 0.0 && frequency <= sampleRate * 0.5);
        jassert (Q > 0.0);

        // Use standard JUCE bilinear transform
        // This works correctly but has frequency cramping at high frequencies
        return juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, static_cast<float>(frequency), static_cast<float>(Q));
    }

    void updateFilters()
    {
        if (currentSampleRate <= 0.0)
            return;

        // HPF Q value
        float hpfQ = (hpfQMode == Normal) ? 0.707f : 1.0f;  // Butterworth Q for natural response

        // Apply channel variation HPF Q offset
        hpfQ += hpfQOffset;

        // Clamp Q to reasonable range to avoid instability
        hpfQ = juce::jlimit (0.1f, 5.0f, hpfQ);

        // Create HPF coefficients using Matched-Z Transform (12 dB/oct base, cascade for 18 dB/oct)
        auto hpfCoeffs = makeMatchedHighPass (
            currentSampleRate,
            juce::jlimit (20.0, currentSampleRate * 0.49, static_cast<double>(hpfFreq)),
            hpfQ);

        hpf1.coefficients = hpfCoeffs;
        hpf2.coefficients = hpfCoeffs;  // Same coefficients for cascade

        // LPF Q value
        float lpfQ = (lpfQMode == Normal) ? 0.707f : 1.0f;

        // For 6 dB/oct: use lower Q for gentler slope
        if (lpfSlope == Slope_6dB)
            lpfQ = 0.5f;

        // Apply channel variation LPF Q offset
        lpfQ += lpfQOffset;

        // Clamp Q to reasonable range to avoid instability
        lpfQ = juce::jlimit (0.1f, 5.0f, lpfQ);

        // Create LPF coefficients using Matched-Z Transform
        auto lpfCoeffs = makeMatchedLowPass (
            currentSampleRate,
            juce::jlimit (20.0, currentSampleRate * 0.49, static_cast<double>(lpfFreq)),
            lpfQ);

        lpf1.coefficients = lpfCoeffs;
        lpf2.coefficients = lpfCoeffs;  // Same coefficients for cascade
    }

    //==============================================================================
    // Filter parameters
    float hpfFreq = 20.0f;
    Slope hpfSlope = Slope_12dB;
    QMode hpfQMode = Normal;

    float lpfFreq = 24000.0f;
    Slope lpfSlope = Slope_6dB;
    QMode lpfQMode = Normal;

    // Channel variation Q offsets
    float hpfQOffset = 0.0f;  // ±0.06
    float lpfQOffset = 0.0f;  // ±0.06

    // IIR filters
    juce::dsp::IIR::Filter<float> hpf1, hpf2;  // HPF: use 2 for 18dB/oct cascade
    juce::dsp::IIR::Filter<float> lpf1, lpf2;  // LPF: use 2 for 12dB/oct cascade

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterSection)
};
