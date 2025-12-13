/*
  ==============================================================================

    ControlCompSectionComponent.h
    GUI component for Control-Comp section (140px column)
    - Threshold knob
    - A/R toggle buttons (Fast/Normal)
    - GR LED meter strip

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"
#include "LEDMeterStrip.h"

class ControlCompSectionComponent : public juce::Component,
                                      private juce::Button::Listener
{
public:
    ControlCompSectionComponent (juce::AudioProcessorValueTreeState& apvts)
        : apvtsRef (apvts), grMeter (8)
    {
        // Threshold knob
        thresholdKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        thresholdKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
        thresholdKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (thresholdKnob);

        thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "ctrlCompThresh", thresholdKnob);

        // Threshold label
        thresholdLabel.setText ("THRESHOLD", juce::dontSendNotification);
        thresholdLabel.setJustificationType (juce::Justification::centred);
        thresholdLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (thresholdLabel);

        // Attack/Release toggle button
        // Parameter order: { "Normal", "Fast" } → OFF (gray) = Normal, ON (orange) = Fast
        arButton.setButtonText ("FAST");
        arButton.setClickingTogglesState (true);
        addAndMakeVisible (arButton);

        arAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "ctrlCompAR", arButton);

        // GR meter
        addAndMakeVisible (grMeter);

        // Active/Inactive button (inverted bypass logic)
        activeButton.setButtonText ("ACTIVE");
        activeButton.setClickingTogglesState (true);
        activeButton.addListener (this);
        addAndMakeVisible (activeButton);

        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "ctrlCompBypass", activeButton);

        // Section label
        sectionLabel.setText ("CLEAN COMP.", juce::dontSendNotification);
        sectionLabel.setJustificationType (juce::Justification::centred);
        sectionLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        sectionLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        addAndMakeVisible (sectionLabel);

        // GR label
        grLabel.setText ("GR", juce::dontSendNotification);
        grLabel.setJustificationType (juce::Justification::centred);
        grLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_DIM);
        grLabel.setFont (juce::FontOptions (10.0f));
        addAndMakeVisible (grLabel);

        // Initialize state
        updateBypassState();
    }

    //==============================================================================
    void buttonClicked (juce::Button* button) override
    {
        if (button == &activeButton)
        {
            updateBypassState();
        }
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

    void paintOverChildren (juce::Graphics& g) override
    {
        // Gray out entire section if bypassed
        bool isBypassed = activeButton.getToggleState();

        if (isBypassed)
        {
            auto bounds = getLocalBounds().toFloat().reduced (2.0f);
            g.setColour (AnalogChannelColors::BG_DARK.withAlpha (0.6f));
            g.fillRoundedRectangle (bounds, 4.0f);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (8);

        // Section label at top
        sectionLabel.setBounds (bounds.removeFromTop (25));
        bounds.removeFromTop (5);

        // Threshold knob
        thresholdLabel.setBounds (bounds.removeFromTop (15));
        thresholdKnob.setBounds (bounds.removeFromTop (80));
        bounds.removeFromTop (10);

        // A/R toggle button (single button)
        arButton.setBounds (bounds.removeFromTop (24));
        bounds.removeFromTop (10);

        // GR meter
        grLabel.setBounds (bounds.removeFromTop (15));
        grMeter.setBounds (bounds.removeFromTop (40).reduced (5, 0));
        bounds.removeFromTop (10);

        // Active button at bottom
        activeButton.setBounds (bounds.removeFromBottom (26));
    }

    //==============================================================================
    // Accessor for parent to update GR meter
    LEDMeterStrip& getGRMeter() { return grMeter; }

private:
    //==============================================================================
    void updateBypassState()
    {
        bool isBypassed = activeButton.getToggleState();
        bool isActive = !isBypassed;

        // Update button label
        activeButton.setButtonText (isActive ? "ACTIVE" : "INACTIVE");

        // Enable/disable controls
        thresholdKnob.setEnabled (isActive);
        arButton.setEnabled (isActive);
        thresholdLabel.setAlpha (isActive ? 1.0f : 0.4f);
        grLabel.setAlpha (isActive ? 1.0f : 0.4f);
        sectionLabel.setAlpha (isActive ? 1.0f : 0.4f);

        repaint();
    }

    //==============================================================================
    juce::AudioProcessorValueTreeState& apvtsRef;

    juce::Slider thresholdKnob;
    juce::Label thresholdLabel;
    juce::ToggleButton arButton;
    LEDMeterStrip grMeter;
    juce::ToggleButton activeButton;
    juce::Label sectionLabel;
    juce::Label grLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlCompSectionComponent)
};
