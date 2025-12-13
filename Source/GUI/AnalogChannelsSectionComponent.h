/*
  ==============================================================================

    AnalogChannelsSectionComponent.h
    GUI component for Analog Channels section (Channel Variation system)

    Copyright (c) 2025 KuramaSound
    Licensed under GPL v3 - see LICENSE file for details

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"

//==============================================================================
/**
    Analog Channels section component.
    Displays channel variation mode and channel pair selection.
*/
class AnalogChannelsSectionComponent : public juce::Component
{
public:
    AnalogChannelsSectionComponent (juce::AudioProcessorValueTreeState& vts)
        : apvts (vts)
    {
        // Section title (like other sections)
        addAndMakeVisible (sectionLabel);
        sectionLabel.setText ("CHANNEL VARIATION", juce::dontSendNotification);
        sectionLabel.setFont (juce::FontOptions (9.0f, juce::Font::bold));
        sectionLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        sectionLabel.setJustificationType (juce::Justification::centred);

        // Mode selector (Off / Stereo / Mono)
        addAndMakeVisible (modeSelector);
        modeSelector.addItem ("Off", 1);
        modeSelector.addItem ("Stereo", 2);
        modeSelector.addItem ("Mono", 3);
        modeSelector.setSelectedId (2, juce::dontSendNotification); // Default: Stereo
        modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            apvts, "channelVariationMode", modeSelector);

        // Channel display label "Channels:" (above display)
        addAndMakeVisible (channelPrefixLabel);
        channelPrefixLabel.setText ("Channels:", juce::dontSendNotification);
        channelPrefixLabel.setFont (juce::FontOptions (9.0f));
        channelPrefixLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        channelPrefixLabel.setJustificationType (juce::Justification::centred);

        // Channel display (shows "1 | 2", "3 | 4", etc.) - centered
        addAndMakeVisible (channelDisplay);
        channelDisplay.setFont (juce::FontOptions (16.0f, juce::Font::bold));
        channelDisplay.setColour (juce::Label::textColourId, AnalogChannelColors::LED_GREEN);
        channelDisplay.setColour (juce::Label::backgroundColourId, AnalogChannelColors::BG_DARK);
        channelDisplay.setJustificationType (juce::Justification::centred);
        channelDisplay.setEditable (false);

        // Channel pair knob (0-23)
        addAndMakeVisible (channelKnob);
        channelKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        channelKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        channelKnob.setRange (0, 23, 1);
        channelKnob.setValue (0, juce::dontSendNotification);
        channelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "channelPair", channelKnob);

        // Update display when mode or channel changes
        modeSelector.onChange = [this] { updateDisplay(); };
        channelKnob.onValueChange = [this] { updateDisplay(); };

        // Initial display update
        updateDisplay();
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (6);

        // Section title at top (like other sections)
        sectionLabel.setBounds (bounds.removeFromTop (12));
        bounds.removeFromTop (8); // Increased spacing to move content down

        // Mode selector (same height as other ComboBoxes)
        modeSelector.setBounds (bounds.removeFromTop (24));
        bounds.removeFromTop (8); // Increased spacing

        // "Channels:" label above display
        channelPrefixLabel.setBounds (bounds.removeFromTop (12));
        bounds.removeFromTop (2); // Small spacing

        // Channel display (centered, full width)
        channelDisplay.setBounds (bounds.removeFromTop (28));
        bounds.removeFromTop (6); // Spacing

        // Channel knob at bottom (centered)
        auto knobArea = bounds.removeFromTop (44);
        int knobSize = 44;
        int knobX = (knobArea.getWidth() - knobSize) / 2;
        channelKnob.setBounds (knobArea.getX() + knobX, knobArea.getY(), knobSize, knobSize);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        // Background with custom color (#626161)
        g.setColour (juce::Colour (0xff626161));
        g.fillRoundedRectangle (bounds.toFloat(), 4.0f);

        // Border
        g.setColour (AnalogChannelColors::BORDER_LIGHT);
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), 4.0f, 1.0f);

        // Title area separator (optional)
        g.setColour (AnalogChannelColors::BORDER_DARK);
        g.drawLine (4.0f, 54.0f, static_cast<float>(bounds.getWidth()) - 4.0f, 54.0f, 1.0f);
    }

private:
    void updateDisplay()
    {
        int mode = modeSelector.getSelectedId() - 1; // 0=Off, 1=Stereo, 2=Mono
        int pair = static_cast<int> (channelKnob.getValue());

        if (mode == 0) // Off
        {
            channelDisplay.setText ("--", juce::dontSendNotification);
            channelDisplay.setColour (juce::Label::textColourId, AnalogChannelColors::LED_OFF);
        }
        else if (mode == 1) // Stereo
        {
            int leftCh = pair * 2 + 1;   // 1, 3, 5, ..., 47
            int rightCh = pair * 2 + 2;  // 2, 4, 6, ..., 48
            channelDisplay.setText (juce::String (leftCh) + " | " + juce::String (rightCh),
                                   juce::dontSendNotification);
            channelDisplay.setColour (juce::Label::textColourId, AnalogChannelColors::LED_GREEN);
        }
        else // Mono
        {
            int ch = pair * 2 + 1;  // 1, 3, 5, ..., 47
            channelDisplay.setText (juce::String (ch) + " | " + juce::String (ch),
                                   juce::dontSendNotification);
            channelDisplay.setColour (juce::Label::textColourId, AnalogChannelColors::LED_YELLOW);
        }
    }

    //==============================================================================
    juce::AudioProcessorValueTreeState& apvts;

    juce::Label sectionLabel;
    juce::ComboBox modeSelector;
    juce::Label channelPrefixLabel;
    juce::Label channelDisplay;
    juce::Slider channelKnob;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> channelAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogChannelsSectionComponent)
};
