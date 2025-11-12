/*
  ==============================================================================

    VolumeSection.h
    Section 8: Output Volume Control

  ==============================================================================
*/

#pragma once

#include "BypassableSection.h"

//==============================================================================
/**
    Final output volume control section.
    Simple gain multiplication.
*/
class VolumeSection : public BypassableSection
{
public:
    VolumeSection() = default;

    //==============================================================================
    /**
        Sets the output gain in decibels.
        @param dB gain from -60 dB to +12 dB
    */
    void setGain (float dB)
    {
        gainDB = dB;
        gainLinear = std::pow (10.0f, dB / 20.0f);
    }

protected:
    //==============================================================================
    float processInternal (float input) override
    {
        return input * gainLinear;
    }

private:
    //==============================================================================
    float gainDB = 0.0f;
    float gainLinear = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VolumeSection)
};
