/*
  ==============================================================================

    ConsoleVolumeBarComponent.h
    GUI component for bottom bar (Console + Volume sections)
    - Console: Algorithm selector + Drive knob + Bypass
    - Volume: Output gain knob

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"

class ConsoleVolumeBarComponent : public juce::Component
{
public:
    ConsoleVolumeBarComponent (juce::AudioProcessorValueTreeState& apvts)
    {
        // Console section label
        consoleLabel.setText ("CONSOLE", juce::dontSendNotification);
        consoleLabel.setJustificationType (juce::Justification::centredLeft);
        consoleLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        consoleLabel.setFont (juce::Font (12.0f, juce::Font::bold));
        addAndMakeVisible (consoleLabel);

        // Console algorithm selector
        consoleAlgoCombo.addItem ("Clean", 1);
        consoleAlgoCombo.addItem ("Pure", 2);
        consoleAlgoCombo.addItem ("Oxford", 3);
        consoleAlgoCombo.addItem ("Essex", 4);
        consoleAlgoCombo.addItem ("USA", 5);
        addAndMakeVisible (consoleAlgoCombo);

        consoleAlgoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            apvts, "consoleAlgo", consoleAlgoCombo);

        // Console drive knob
        consoleDriveKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        consoleDriveKnob.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        consoleDriveKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (consoleDriveKnob);

        consoleDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "consoleDrive", consoleDriveKnob);

        // Console drive label
        consoleDriveLabel.setText ("DRIVE", juce::dontSendNotification);
        consoleDriveLabel.setJustificationType (juce::Justification::centred);
        consoleDriveLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        consoleDriveLabel.setFont (juce::Font (9.0f));
        addAndMakeVisible (consoleDriveLabel);

        // Console bypass button removed (Clean = bypass)

        // Volume section label
        volumeLabel.setText ("VOLUME", juce::dontSendNotification);
        volumeLabel.setJustificationType (juce::Justification::centredLeft);
        volumeLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        volumeLabel.setFont (juce::Font (12.0f, juce::Font::bold));
        addAndMakeVisible (volumeLabel);

        // Output gain knob
        outputGainKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        outputGainKnob.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        outputGainKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (outputGainKnob);

        outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "outputGain", outputGainKnob);

        // Output gain label
        outputGainLabel.setText ("OUTPUT", juce::dontSendNotification);
        outputGainLabel.setJustificationType (juce::Justification::centred);
        outputGainLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        outputGainLabel.setFont (juce::Font (9.0f));
        addAndMakeVisible (outputGainLabel);
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour (AnalogChannelColors::PANEL_BG);
        g.fillRect (bounds);

        // Divider line between Console and Volume
        float dividerX = bounds.getWidth() * 0.6f;
        g.setColour (AnalogChannelColors::BORDER_LIGHT);
        g.drawLine (dividerX, 5.0f, dividerX, bounds.getHeight() - 5.0f, 1.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (8, 3);

        // Console section (left side)
        auto consoleArea = bounds.removeFromLeft (350);

        consoleLabel.setBounds (consoleArea.removeFromTop (12));

        auto consoleControlsArea = consoleArea;

        // Algorithm selector (centered vertically)
        consoleAlgoCombo.setBounds (consoleControlsArea.removeFromLeft (100).withTrimmedTop(3).withHeight (22));
        consoleControlsArea.removeFromLeft (8);

        // Drive label inline (centered vertically)
        consoleDriveLabel.setBounds (consoleControlsArea.removeFromLeft (40).withTrimmedTop(3).withHeight (22));
        consoleControlsArea.removeFromLeft (4);

        // Drive knob (larger, 50px diameter with textbox on right, vertically centered with trim only)
        consoleDriveKnob.setBounds (consoleControlsArea.removeFromLeft (130).withTrimmedTop(3));

        // Volume section (right side, aligned to right edge)
        auto volumeArea = bounds; // Use remaining space

        volumeLabel.setBounds (volumeArea.removeFromTop (12));

        auto volumeControlsArea = volumeArea;

        // Output label inline (centered vertically)
        outputGainLabel.setBounds (volumeControlsArea.removeFromLeft (55).withTrimmedTop(3).withHeight (22));
        volumeControlsArea.removeFromLeft (4);

        // Output gain knob (larger, 50px diameter with textbox on right, vertically centered with trim only)
        outputGainKnob.setBounds (volumeControlsArea.removeFromLeft (130).withTrimmedTop(3));
    }

private:
    //==============================================================================
    // Console section
    juce::Label consoleLabel;
    juce::ComboBox consoleAlgoCombo;
    juce::Slider consoleDriveKnob;
    juce::Label consoleDriveLabel;

    // Volume section
    juce::Label volumeLabel;
    juce::Slider outputGainKnob;
    juce::Label outputGainLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> consoleAlgoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> consoleDriveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleVolumeBarComponent)
};
