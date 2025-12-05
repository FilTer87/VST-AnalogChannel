/*
  ==============================================================================

    PluginEditor.h
    AnalogChannel VST3 Channel Strip Plugin - GUI

    Copyright (c) 2025 KuramaSound

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/AnalogChannelLookAndFeel.h"
#include "GUI/PeakMeter.h"
#include "GUI/LEDMeterStrip.h"
#include "GUI/PreInputSectionComponent.h"
#include "GUI/FiltersSectionComponent.h"
#include "GUI/ControlCompSectionComponent.h"
#include "GUI/LowDynamicSectionComponent.h"
#include "GUI/EQSectionComponent.h"
#include "GUI/StyleCompSectionComponent.h"
#include "GUI/OutStageSectionComponent.h"
#include "GUI/ConsoleSectionComponent.h"
#include "GUI/AnalogChannelsSectionComponent.h"
#include "GUI/VolumeSectionComponent.h"
#include "GUI/Common/PluginHeaderBar.h"

//==============================================================================
/**
    AnalogChannel Plugin Editor
    Hardware-inspired GUI with knobs, LED meters, and clean layout
*/
class AnalogChannelAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                               private juce::Timer,
                                               private juce::AudioProcessorValueTreeState::Listener
{
public:
    AnalogChannelAudioProcessorEditor (AnalogChannelAudioProcessor&);
    ~AnalogChannelAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Timer callback for updating meters
    void timerCallback() override;

    // Menu callbacks
    void populateMenu (juce::PopupMenu& menu);
    void handleMenuResult (int result);

    // Apply zoom scale to plugin window
    void applyZoomScale (float scale);

    // Listen to parameter changes (for zoom preset recall)
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    //==============================================================================
    // Processor reference
    AnalogChannelAudioProcessor& audioProcessor;

    // Custom Look & Feel
    AnalogChannelLookAndFeel AnalogChannelLAF;

    // Header bar (reusable component)
    PluginHeaderBar headerBar;

    // Peak Meters (input/output, L/R)
    PeakMeter inputMeterLeft, inputMeterRight;
    PeakMeter outputMeterLeft, outputMeterRight;

    // Section components
    PreInputSectionComponent preInputSection;
    FiltersSectionComponent filtersSection;
    ControlCompSectionComponent controlCompSection;
    LowDynamicSectionComponent lowDynamicSection;
    EQSectionComponent eqSection;
    StyleCompSectionComponent styleCompSection;
    ConsoleSectionComponent consoleSection;
    OutStageSectionComponent outStageSection;
    AnalogChannelsSectionComponent analogChannelsSection;
    VolumeSectionComponent volumeSection;

    // Logo image for About dialog
    juce::Image bannerLogoImage;

    // Current zoom scale (0.75, 1.0, 1.25, 1.5)
    float currentZoomScale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogChannelAudioProcessorEditor)
};
