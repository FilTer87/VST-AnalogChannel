/*
  ==============================================================================

    VolumeSectionComponent.h
    GUI component for Volume section
    - Output gain knob

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"

class VolumeSectionComponent : public juce::Component
{
public:
    VolumeSectionComponent (juce::AudioProcessorValueTreeState& apvts)
    {
        // Output gain knob
        outputGainKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        outputGainKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        outputGainKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (outputGainKnob);

        outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "outputGain", outputGainKnob);

        // Output gain label (compact)
        outputGainLabel.setText ("OUTPUT", juce::dontSendNotification);
        outputGainLabel.setJustificationType (juce::Justification::centred);
        outputGainLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        outputGainLabel.setFont (juce::FontOptions (9.0f));
        addAndMakeVisible (outputGainLabel);
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background panel
        g.setColour (AnalogChannelColors::PANEL_BG);
        g.fillRoundedRectangle (bounds.reduced (2.0f), 4.0f);

        // Border
        g.setColour (AnalogChannelColors::BORDER_LIGHT);
        g.drawRoundedRectangle (bounds.reduced (2.0f), 4.0f, 1.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (8);

        // Compact layout: just label + knob (no section title)
        outputGainLabel.setBounds (bounds.removeFromTop (15));
        bounds.removeFromTop (2); // Small spacing
        outputGainKnob.setBounds (bounds.removeFromTop (80));
    }

private:
    //==============================================================================
    juce::Slider outputGainKnob;
    juce::Label outputGainLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VolumeSectionComponent)
};
