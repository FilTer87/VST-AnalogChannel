/*
  ==============================================================================

    OutStageSection.h
    Section 7: Output Stage
    Algorithms: Clean, Pure (PurestDrive), Tape (ToTape8), Tube (Tube2),
                Hard Clip (FinalClip), Soft Clip (ClipSoftly)

    Reuses algorithms from Pre-Input section plus two clippers.

  ==============================================================================
*/

#pragma once

#include "BypassableSection.h"
#include "../Algorithms/PurestDrive.h"
#include "../Algorithms/ToTape8.h"
#include "../Algorithms/Tube2.h"
#include "../Algorithms/FinalClip.h"
#include "../Algorithms/ClipSoftly.h"

//==============================================================================
/**
    OutStage section - Output saturation and clipping.
    Six modes: Clean, Pure, Tape, Tube, Hard Clip, Soft Clip.
*/
class OutStageSection : public BypassableSection
{
public:
    enum Algorithm
    {
        Clean = 0,
        Pure = 1,
        Tape = 2,
        Tube = 3,
        HardClip = 4,
        SoftClip = 5
    };

    OutStageSection() = default;

    //==============================================================================
    void setSampleRate (double sampleRate) override
    {
        BypassableSection::setSampleRate (sampleRate);
        purestDrive.setSampleRate (sampleRate);
        toTape8.setSampleRate (sampleRate);
        tube2.setSampleRate (sampleRate);
        finalClip.setSampleRate (sampleRate);
        clipSoftly.setSampleRate (sampleRate);
    }

    void reset() override
    {
        purestDrive.reset();
        toTape8.reset();
        tube2.reset();
        finalClip.reset();
        clipSoftly.reset();
    }

    //==============================================================================
    /**
        Sets the saturation/clipping algorithm.
        @param algo algorithm to use (Clean, Pure, Tape, Tube, HardClip, SoftClip)
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

            case HardClip:
                // FinalClip hard clipper with drive compensation
                {
                    float driven = input * driveLinear;
                    float processed = finalClip.process (driven);
                    return processed / driveLinear; // Compensate drive
                }

            case SoftClip:
                // ClipSoftly soft clipper with drive compensation
                {
                    float driven = input * driveLinear;
                    float processed = clipSoftly.process (driven);
                    return processed / driveLinear; // Compensate drive
                }

            default:
                return input;
        }
    }

private:
    //==============================================================================
    Algorithm currentAlgorithm = Clean;  // Default: Clean
    float driveDB = 0.0f;
    float driveLinear = 1.0f;

    // Algorithms (reused from Pre-Input)
    PurestDrive purestDrive;
    ToTape8 toTape8;
    Tube2 tube2;

    // Clippers (new)
    FinalClip finalClip;
    ClipSoftly clipSoftly;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutStageSection)
};
