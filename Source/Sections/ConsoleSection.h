/*
  ==============================================================================

    ConsoleSection.h
    Section 6: Console Emulation
    Algorithms: Clean, Pure (PurestConsole3), Oxford/Essex/USA (Channel8)

    Console emulations:
    - Clean: Bypass (no processing)
    - Pure: PurestConsole3Channel (Airwindows port)
    - Oxford: SSL (Channel8 - tight, clean, fast transients)
    - Essex: Neve (Channel8 - warm, smooth, rounded transients)
    - USA: API (Channel8 - punchy, preserves low end)

    Channel8 algorithm: Copyright (c) 2016 airwindows, MIT license
    Professional 4-stage console emulation with adaptive filtering,
    dual saturation, slew rate limiting, and dithering.

  ==============================================================================
*/

#pragma once

#include "BypassableSection.h"
#include "../Algorithms/PurestConsole3Channel.h"
#include "../Algorithms/Channel8Console.h"

//==============================================================================
/**
    Console section - Analog console emulation.
    Five modes: Clean, Pure, Oxford (SSL), Essex (Neve), USA (API).
*/
class ConsoleSection : public BypassableSection
{
public:
    enum Algorithm
    {
        Clean = 0,  // Bypass
        Pure = 1,   // PurestConsole3Channel (faithful AirWindows port)
        Oxford = 2, // SSL (Channel8)
        Essex = 3,  // Neve (Channel8)
        USA = 4     // API (Channel8)
    };

    ConsoleSection()
    {
        // Configure Channel8 console types
        consoleSSL.setConsoleType (Channel8Console::SSL);
        consoleNeve.setConsoleType (Channel8Console::Neve);
        consoleAPI.setConsoleType (Channel8Console::API);
    }

    //==============================================================================
    void setSampleRate (double sampleRate) override
    {
        BypassableSection::setSampleRate (sampleRate);
        pureConsole.setSampleRate (sampleRate);
        consoleSSL.setSampleRate (sampleRate);
        consoleNeve.setSampleRate (sampleRate);
        consoleAPI.setSampleRate (sampleRate);
    }

    void reset() override
    {
        pureConsole.reset();
        consoleSSL.reset();
        consoleNeve.reset();
        consoleAPI.reset();
    }

    //==============================================================================
    /**
        Sets the console algorithm.
        @param algo Clean, Pure, Oxford, Essex, or USA
    */
    void setAlgorithm (Algorithm algo)
    {
        currentAlgorithm = algo;
    }

    /**
        Sets the drive amount in decibels.
        Drive is applied before console processing and compensated after.
        @param dB drive from -18 dB to +18 dB
    */
    void setDrive (float dB)
    {
        driveDB = dB;
        driveGain = std::pow (10.0f, dB / 20.0f);
    }

protected:
    //==============================================================================
    float processInternal (float input) override
    {
        if (currentAlgorithm == Clean)
        {
            // Clean: bypass
            return input;
        }

        // Apply drive (increase level before console)
        float driven = input * driveGain;

        // Process through console algorithm
        float processed;
        switch (currentAlgorithm)
        {
            case Pure:
                processed = pureConsole.process (driven);
                break;

            case Oxford:
                processed = consoleSSL.process (driven);
                break;

            case Essex:
                processed = consoleNeve.process (driven);
                break;

            case USA:
                processed = consoleAPI.process (driven);
                break;

            default: // Clean
                processed = driven;
                break;
        }

        // Compensate drive (decrease level after console)
        return processed / driveGain;
    }

private:
    //==============================================================================
    PurestConsole3Channel pureConsole;
    Channel8Console consoleSSL;   // Channel8 configured as SSL
    Channel8Console consoleNeve;  // Channel8 configured as Neve
    Channel8Console consoleAPI;   // Channel8 configured as API

    Algorithm currentAlgorithm = Clean;  // Default: Clean (bypass)
    float driveDB = 0.0f;
    float driveGain = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleSection)
};
