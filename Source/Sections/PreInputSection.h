/*
  ==============================================================================

    PreInputSection.h
    Section 1: Pre-Input Saturation/Drive
    Algorithms: Clean, Pure (PurestDrive), Tape (TODO), Tube (TODO)

  ==============================================================================
*/

#pragma once

#include "BypassableSection.h"
#include "../Algorithms/PurestDrive.h"
#include "../Algorithms/ToTape8.h"
#include "../Algorithms/Tube2.h"

//==============================================================================
/**
    Pre-Input section - Saturation and drive.
    Provides Clean (gain only) and Pure (PurestDrive saturation).
*/
class PreInputSection : public BypassableSection
{
public:
    enum Algorithm
    {
        Clean = 0,
        Pure = 1,
        Tape = 2,
        Tube = 3
    };

    PreInputSection() = default;

    //==============================================================================
    void setSampleRate (double sampleRate) override
    {
        BypassableSection::setSampleRate (sampleRate);
        purestDrive.setSampleRate (sampleRate);
        toTape8.setSampleRate (sampleRate);
        tube2.setSampleRate (sampleRate);
    }

    void reset() override
    {
        purestDrive.reset();
        toTape8.reset();
        tube2.reset();
    }

    //==============================================================================
    /**
        Sets the saturation algorithm.
        @param algo algorithm to use (Clean, Pure, Tape, Tube)
    */
    void setAlgorithm (Algorithm algo)
    {
        currentAlgorithm = algo;
    }

    /**
        Sets the drive amount in decibels.
        @param dB drive from -18 dB to +18 dB
    */
    void setDrive (float dB)
    {
        driveDB = dB;
        driveLinear = std::pow (10.0f, dB / 20.0f);
    }

    /**
        Sets the channel index for PRNG seed initialization.
        This ensures L and R channels have independent random sequences for flutter.
        @param channelIdx 0 for left, 1 for right
    */
    void setChannelIndex (int channelIdx)
    {
        // Use large prime offset (1000000007) to ensure completely different PRNG sequences
        uint32_t seed = 17 + static_cast<uint32_t>(channelIdx) * 1000000007;
        toTape8.setPRNGSeed (seed);
        tube2.setPRNGSeed (seed);
    }

protected:
    //==============================================================================
    float processInternal (float input) override
    {
        switch (currentAlgorithm)
        {
            case Clean:
                // Simple gain - no saturation
                return input * driveLinear;

            case Pure:
                // PurestDrive saturation
                return purestDrive.process (input, driveDB);

            case Tape:
                // ToTape8 tape saturation
                return toTape8.process (input, driveDB);

            case Tube:
                // Tube2 tube saturation
                return tube2.process (input, driveDB);

            default:
                return input;
        }
    }

private:
    //==============================================================================
    Algorithm currentAlgorithm = Pure;  // Default: Pure
    float driveDB = 0.0f;
    float driveLinear = 1.0f;

    // Algorithms
    PurestDrive purestDrive;
    ToTape8 toTape8;
    Tube2 tube2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreInputSection)
};
