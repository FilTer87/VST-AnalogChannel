/*
  ==============================================================================

    EQSection.h
    Section 4: EQ (Low Shelf + 2x Bell + High Shelf)
    Bell filters: API 550-style dynamic Q
    Shelves: Baxandall2 (TODO - waiting for source)

  ==============================================================================
*/

#pragma once

#include "BypassableSection.h"
#include "../Algorithms/BellFilter.h"
#include "../Algorithms/Baxandall2.h"

//==============================================================================
/**
    EQ section with shelves and parametric bells.
    Signal flow: Bass Shelf → Bell1 → Bell2 → Treble Shelf
*/
class EQSection : public BypassableSection
{
public:
    EQSection() = default;

    //==============================================================================
    void setSampleRate (double sampleRate) override
    {
        BypassableSection::setSampleRate (sampleRate);
        baxandall.setSampleRate (sampleRate);
        bell1.setSampleRate (sampleRate);
        bell2.setSampleRate (sampleRate);
    }

    void reset() override
    {
        baxandall.reset();
        bell1.reset();
        bell2.reset();
    }

    //==============================================================================
    /**
        Set bass shelf gain.
        @param dB gain from -15 to +15 dB
    */
    void setBassShelf (float dB)
    {
        baxandall.setBass (dB);
    }

    /**
        Set treble shelf gain.
        @param dB gain from -15 to +15 dB
    */
    void setTrebleShelf (float dB)
    {
        baxandall.setTreble (dB);
    }

    /**
        Set bass shelf frequency.
        @param hz frequency in Hz
    */
    void setBassShelfFreq (float hz)
    {
        baxandall.setBassFreq (hz);
    }

    /**
        Set treble shelf frequency.
        @param hz frequency in Hz
    */
    void setTrebleShelfFreq (float hz)
    {
        baxandall.setTrebleFreq (hz);
    }

    /**
        Set bell 1 parameters.
        @param freqIndex index into frequency table (0-9)
        @param gainDB gain from -12 to +12 dB
    */
    void setBell1 (int freqIndex, float gainDB)
    {
        float freq = getFrequencyFromIndex (freqIndex);
        bell1.setParameters (freq, gainDB);
    }

    /**
        Set bell 2 parameters.
        @param freqIndex index into frequency table (0-9)
        @param gainDB gain from -12 to +12 dB
    */
    void setBell2 (int freqIndex, float gainDB)
    {
        float freq = getFrequencyFromIndex (freqIndex);
        bell2.setParameters (freq, gainDB);
    }

    /**
        Set bell 1 parameters with channel variation offsets.
        @param freqIndex index into frequency table (0-14)
        @param gainDB gain from -12 to +12 dB
        @param freqOffset frequency offset in Hz (for channel variation)
        @param gainOffset gain offset in dB (for channel variation)
        @param qOffset Q offset (for channel variation)
    */
    void setBell1WithVariation (int freqIndex, float gainDB, float freqOffset, float gainOffset, float qOffset)
    {
        float baseFreq = getFrequencyFromIndex (freqIndex);
        bell1.setParameters (baseFreq + freqOffset, gainDB + gainOffset);
        bell1.setQOffset (qOffset);
    }

    /**
        Set bell 2 parameters with channel variation offsets.
        @param freqIndex index into frequency table (0-14)
        @param gainDB gain from -12 to +12 dB
        @param freqOffset frequency offset in Hz (for channel variation)
        @param gainOffset gain offset in dB (for channel variation)
        @param qOffset Q offset (for channel variation)
    */
    void setBell2WithVariation (int freqIndex, float gainDB, float freqOffset, float gainOffset, float qOffset)
    {
        float baseFreq = getFrequencyFromIndex (freqIndex);
        bell2.setParameters (baseFreq + freqOffset, gainDB + gainOffset);
        bell2.setQOffset (qOffset);
    }

protected:
    //==============================================================================
    float processInternal (float input) override
    {
        float output = input;

        // Baxandall2 processes both bass and treble together
        output = baxandall.process (output);

        // Bell 1
        output = bell1.process (output);

        // Bell 2
        output = bell2.process (output);

        return output;
    }

private:
    //==============================================================================
    /**
        Convert frequency index to actual frequency in Hz.
        Fixed frequency steps: [200, 300, 400, 500, 750, 1k, 2k, 3.5k, 5k, 7k]
    */
    float getFrequencyFromIndex (int index)
    {
        const float frequencies[] = {
            50.0f, 100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 700.0f, 900.0f,
            1400.0f, 2400.0f, 3500.0f, 5000.0f, 7500.0f, 10000.0f, 13000.0f
        };

        index = juce::jlimit (0, 14, index);
        return frequencies[index];
    }

    //==============================================================================
    Baxandall2 baxandall;
    BellFilter bell1, bell2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQSection)
};
