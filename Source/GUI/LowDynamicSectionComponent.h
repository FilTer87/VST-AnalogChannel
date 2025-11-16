/*
  ==============================================================================

    LowDynamicSectionComponent.h
    GUI component for Low Dynamic section
    - Threshold knob
    - Ratio knob (bipolar: -10 to +10, Expand | Lift)
    - Fast/Normal toggle button
    - Dynamic label showing EXPAND/OFF/LIFT

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"

class LowDynamicSectionComponent : public juce::Component,
                                     private juce::Button::Listener,
                                     private juce::Slider::Listener
{
public:
    LowDynamicSectionComponent (juce::AudioProcessorValueTreeState& apvts)
        : apvtsRef (apvts)
    {
        // Threshold knob (small rotary like EQ MidCut)
        thresholdKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        thresholdKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 45, 16);
        thresholdKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (thresholdKnob);

        thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "lowDynThresh", thresholdKnob);

        // Threshold label (abbreviated)
        thresholdLabel.setText ("Thr.", juce::dontSendNotification);
        thresholdLabel.setJustificationType (juce::Justification::centred);
        thresholdLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        thresholdLabel.setFont (juce::FontOptions (9.0f));
        addAndMakeVisible (thresholdLabel);

        // Mix knob (dry/wet)
        mixKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        mixKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 45, 16);
        mixKnob.setTextValueSuffix (" %");
        addAndMakeVisible (mixKnob);

        mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "lowDynMix", mixKnob);

        // Mix label
        mixLabel.setText ("Mix", juce::dontSendNotification);
        mixLabel.setJustificationType (juce::Justification::centred);
        mixLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        mixLabel.setFont (juce::FontOptions (9.0f));
        addAndMakeVisible (mixLabel);

        // Ratio knob (bipolar)
        ratioKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        ratioKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
        ratioKnob.addListener (this);  // Listen for value changes to update dynamic label
        addAndMakeVisible (ratioKnob);

        ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "lowDynRatio", ratioKnob);

        // Ratio dynamic label (shows current mode: EXPAND/OFF/LIFT) - now above knob instead of static label
        ratioDynamicLabel.setText ("OFF", juce::dontSendNotification);
        ratioDynamicLabel.setJustificationType (juce::Justification::centred);
        ratioDynamicLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        ratioDynamicLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        addAndMakeVisible (ratioDynamicLabel);

        // Fast/Normal toggle button
        fastButton.setButtonText ("FAST");
        fastButton.setClickingTogglesState (true);
        addAndMakeVisible (fastButton);

        fastAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "lowDynFast", fastButton);

        // Active/Inactive button (inverted bypass logic)
        activeButton.setButtonText ("ACTIVE");
        activeButton.setClickingTogglesState (true);
        activeButton.addListener (this);
        addAndMakeVisible (activeButton);

        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "lowDynBypass", activeButton);

        // Section label
        sectionLabel.setText ("LOW DYNAMIC", juce::dontSendNotification);
        sectionLabel.setJustificationType (juce::Justification::centred);
        sectionLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        sectionLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        addAndMakeVisible (sectionLabel);

        // Initialize state
        updateBypassState();
        updateRatioDynamicLabel();
    }

    //==============================================================================
    void buttonClicked (juce::Button* button) override
    {
        if (button == &activeButton)
        {
            updateBypassState();
        }
    }

    void sliderValueChanged (juce::Slider* slider) override
    {
        if (slider == &ratioKnob)
        {
            updateRatioDynamicLabel();
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

        // Bypass overlay (dark semi-transparent layer when bypassed)
        bool isBypassed = activeButton.getToggleState();
        if (isBypassed)
        {
            g.setColour (AnalogChannelColors::BG_DARK.withAlpha (0.6f));
            g.fillRoundedRectangle (bounds.reduced (2.0f), 4.0f);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (8);

        // Section label (top)
        sectionLabel.setBounds (bounds.removeFromTop (16));
        bounds.removeFromTop (4);

        // Ratio dynamic label (EXPAND/OFF/LIFT) above knob, centered
        ratioDynamicLabel.setBounds (bounds.removeFromTop (14));
        bounds.removeFromTop (2);

        // Ratio knob (MEDIUM size, centered)
        ratioKnob.setBounds (bounds.removeFromTop (70).withSizeKeepingCentre (60, 70));
        bounds.removeFromTop (4);

        // Fast button (above Thr/Mix knobs, full width)
        fastButton.setBounds (bounds.removeFromTop (24).reduced (10, 0));
        bounds.removeFromTop (4);

        // Threshold and Mix labels row (aligned above knobs)
        auto labelsRow = bounds.removeFromTop (12);
        thresholdLabel.setBounds (labelsRow.removeFromLeft (50));
        labelsRow.removeFromLeft (10);  // Spacing
        mixLabel.setBounds (labelsRow.removeFromRight (50));
        bounds.removeFromTop (2);

        // Threshold (left) and Mix (right) knobs row
        auto knobsRow = bounds.removeFromTop (60);
        thresholdKnob.setBounds (knobsRow.removeFromLeft (50).withSizeKeepingCentre (50, 60));
        knobsRow.removeFromLeft (10);  // Spacing
        mixKnob.setBounds (knobsRow.removeFromRight (50).withSizeKeepingCentre (50, 60));

        bounds.removeFromTop (4);

        // Active button (bypass, bottom - full width with small margins)
        activeButton.setBounds (bounds.removeFromBottom (24).reduced (4, 0));
    }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState& apvtsRef;

    // Components
    juce::Slider thresholdKnob;
    juce::Slider ratioKnob;
    juce::Slider mixKnob;
    juce::ToggleButton fastButton;
    juce::ToggleButton activeButton;

    juce::Label thresholdLabel;
    juce::Label mixLabel;
    juce::Label ratioDynamicLabel;  // Dynamic: EXPAND/OFF/LIFT (now above ratio knob)
    juce::Label sectionLabel;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fastAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    //==============================================================================
    void updateBypassState()
    {
        // CORRECT LOGIC (same as Filters section):
        // activeButton.getToggleState() == true → Bypassed (parameter = true)
        // activeButton.getToggleState() == false → Active (parameter = false)
        bool isBypassed = activeButton.getToggleState();
        bool isActive = !isBypassed;

        // Update button label
        activeButton.setButtonText (isActive ? "ACTIVE" : "INACTIVE");

        // Enable/disable controls (blocks interaction)
        thresholdKnob.setEnabled (isActive);
        ratioKnob.setEnabled (isActive);
        mixKnob.setEnabled (isActive);
        fastButton.setEnabled (isActive);

        // Dim ALL components when bypassed (knobs + labels + buttons)
        float controlAlpha = isActive ? 1.0f : 0.4f;
        thresholdKnob.setAlpha (controlAlpha);
        ratioKnob.setAlpha (controlAlpha);
        mixKnob.setAlpha (controlAlpha);
        fastButton.setAlpha (controlAlpha);
        thresholdLabel.setAlpha (controlAlpha);
        mixLabel.setAlpha (controlAlpha);
        ratioDynamicLabel.setAlpha (controlAlpha);
        sectionLabel.setAlpha (controlAlpha);
        activeButton.setAlpha (controlAlpha);

        repaint();
    }

    void updateRatioDynamicLabel()
    {
        float ratio = static_cast<float> (ratioKnob.getValue());

        if (ratio < -0.1f)
        {
            ratioDynamicLabel.setText ("EXPAND", juce::dontSendNotification);
            ratioDynamicLabel.setColour (juce::Label::textColourId, AnalogChannelColors::KNOB_INDICATOR);  // Orange for expansion
        }
        else if (ratio > 0.1f)
        {
            ratioDynamicLabel.setText ("LIFT", juce::dontSendNotification);
            ratioDynamicLabel.setColour (juce::Label::textColourId, AnalogChannelColors::LED_GREEN);  // Green for lift
        }
        else
        {
            ratioDynamicLabel.setText ("OFF", juce::dontSendNotification);
            ratioDynamicLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_DIM);  // Gray for off
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowDynamicSectionComponent)
};
