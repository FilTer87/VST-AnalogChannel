/*
  ==============================================================================

    FiltersSectionComponent.h
    GUI component for Filters section
    - HPF: Frequency knob + Slope toggle (12/18 dB/oct) + Q toggle (Bump)
    - LPF: Frequency knob + Slope toggle (6/12 dB/oct) + Q toggle (Bump)
    - Bypass button

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"

class FiltersSectionComponent : public juce::Component,
                                  private juce::Button::Listener
{
public:
    FiltersSectionComponent (juce::AudioProcessorValueTreeState& apvts)
        : apvtsRef (apvts)
    {
        // HPF label
        hpfLabel.setText ("HPF", juce::dontSendNotification);
        hpfLabel.setJustificationType (juce::Justification::centredLeft);
        hpfLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (hpfLabel);

        // HPF knob
        hpfKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        hpfKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 18);
        hpfKnob.setTextValueSuffix (" Hz");
        addAndMakeVisible (hpfKnob);

        hpfAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "hpfFreq", hpfKnob);

        // HPF Slope toggle button (12 dB/oct ↔ 18 dB/oct)
        hpfSlopeButton.setButtonText ("12 dB/oct");
        hpfSlopeButton.setClickingTogglesState (true);
        hpfSlopeButton.addListener (this);
        addAndMakeVisible (hpfSlopeButton);

        hpfSlopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "hpfSlope", hpfSlopeButton);

        // HPF Q toggle button (Bump)
        hpfQButton.setButtonText ("Bump");
        hpfQButton.setClickingTogglesState (true);
        addAndMakeVisible (hpfQButton);

        hpfQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "hpfQ", hpfQButton);

        // LPF label
        lpfLabel.setText ("LPF", juce::dontSendNotification);
        lpfLabel.setJustificationType (juce::Justification::centredLeft);
        lpfLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (lpfLabel);

        // LPF knob
        lpfKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        lpfKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 18);
        lpfKnob.setTextValueSuffix (" Hz");
        addAndMakeVisible (lpfKnob);

        lpfAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "lpfFreq", lpfKnob);

        // LPF Slope toggle button (6 dB/oct ↔ 12 dB/oct)
        lpfSlopeButton.setButtonText ("6 dB/oct");
        lpfSlopeButton.setClickingTogglesState (true);
        lpfSlopeButton.addListener (this);
        addAndMakeVisible (lpfSlopeButton);

        lpfSlopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "lpfSlope", lpfSlopeButton);

        // LPF Q toggle button (Bump)
        lpfQButton.setButtonText ("Bump");
        lpfQButton.setClickingTogglesState (true);
        addAndMakeVisible (lpfQButton);

        lpfQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "lpfQ", lpfQButton);

        // POST button (small, right-aligned)
        postButton.setButtonText ("");  // Empty, we draw custom in paintOverChildren
        postButton.setClickingTogglesState (true);
        postButton.addListener (this);
        // Make button background transparent so we can draw custom style
        postButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        postButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (postButton);

        postAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "filtersPost", postButton);

        // Active/Inactive button
        activeButton.setButtonText ("ACTIVE");
        activeButton.setClickingTogglesState (true);
        activeButton.addListener (this);
        addAndMakeVisible (activeButton);

        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "filtersBypass", activeButton);

        // Section label
        sectionLabel.setText ("FILTERS", juce::dontSendNotification);
        sectionLabel.setJustificationType (juce::Justification::centred);
        sectionLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        sectionLabel.setFont (juce::Font (11.0f, juce::Font::bold));
        addAndMakeVisible (sectionLabel);

        // Initialize button labels based on parameter values
        updateSlopeButtonLabels();
        updateBypassState();
    }

    //==============================================================================
    void buttonClicked (juce::Button* button) override
    {
        if (button == &hpfSlopeButton)
        {
            hpfSlopeButton.setButtonText (hpfSlopeButton.getToggleState() ? "18 dB/oct" : "12 dB/oct");
        }
        else if (button == &lpfSlopeButton)
        {
            // FIXED: toggle OFF (gray) = 6 dB/oct, toggle ON (orange) = 12 dB/oct
            lpfSlopeButton.setButtonText (lpfSlopeButton.getToggleState() ? "12 dB/oct" : "6 dB/oct");
        }
        else if (button == &activeButton)
        {
            updateBypassState();
        }
        else if (button == &postButton)
        {
            repaint();  // Trigger repaint for color change
        }
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background panel with custom color (#4c4c4c)
        g.setColour (juce::Colour (0xff4c4c4c));
        g.fillRoundedRectangle (bounds.reduced (2.0f), 4.0f);

        // Border
        g.setColour (AnalogChannelColors::BORDER_LIGHT);
        g.drawRoundedRectangle (bounds.reduced (2.0f), 4.0f, 1.0f);
    }

    void paintOverChildren (juce::Graphics& g) override
    {
        // Custom paint for POST button (yellow when active)
        auto postBounds = postButton.getBounds().toFloat();
        bool isPostActive = postButton.getToggleState();

        if (isPostActive)
        {
            // Yellow background when active
            g.setColour (juce::Colour (0xffFFD966).withAlpha (0.9f));  // Soft yellow
            g.fillRoundedRectangle (postBounds, 2.0f);
        }
        else
        {
            // Gray background when inactive
            g.setColour (juce::Colour (0xff4a4a4a));
            g.fillRoundedRectangle (postBounds, 2.0f);
        }

        // Border
        g.setColour (AnalogChannelColors::BORDER_DARK);
        g.drawRoundedRectangle (postBounds, 2.0f, 1.0f);

        // Arrow symbol (using > instead of unicode arrow for better compatibility)
        g.setColour (isPostActive ? juce::Colour (0xff1a1a1a) : AnalogChannelColors::TEXT_MAIN);
        g.setFont (juce::Font (10.0f, juce::Font::plain));
        g.drawText ("Post >", postBounds, juce::Justification::centred);

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
        sectionLabel.setBounds (bounds.removeFromTop (20));
        bounds.removeFromTop (4);

        // HPF section (label inline with knob)
        auto hpfArea = bounds.removeFromTop (60);
        hpfLabel.setBounds (hpfArea.removeFromLeft (30));
        hpfKnob.setBounds (hpfArea);

        // HPF buttons (Slope + Q in single row) - minimal width for text
        auto hpfButtonsArea = bounds.removeFromTop (22);
        hpfSlopeButton.setBounds (hpfButtonsArea.removeFromLeft (65));  // Just enough for "18 dB/oct"
        hpfButtonsArea.removeFromLeft (1);  // Minimal spacing
        hpfQButton.setBounds (hpfButtonsArea.removeFromLeft (45));  // Just enough for "Bump"
        bounds.removeFromTop (6);

        // LPF section (label inline with knob)
        auto lpfArea = bounds.removeFromTop (60);
        lpfLabel.setBounds (lpfArea.removeFromLeft (30));
        lpfKnob.setBounds (lpfArea);

        // LPF buttons (Slope + Q in single row) - minimal width for text
        auto lpfButtonsArea = bounds.removeFromTop (22);
        lpfSlopeButton.setBounds (lpfButtonsArea.removeFromLeft (65));  // Just enough for "12 dB/oct"
        lpfButtonsArea.removeFromLeft (1);  // Minimal spacing
        lpfQButton.setBounds (lpfButtonsArea.removeFromLeft (45));  // Just enough for "Bump"
        bounds.removeFromTop (8);

        // POST button (small, right-aligned above ACTIVE button)
        auto postButtonArea = bounds.removeFromTop (20);  // Reduced height
        postButton.setBounds (postButtonArea.removeFromRight (32).withTrimmedTop (0));  // Small width, right-aligned
        bounds.removeFromTop (4);  // Small gap before ACTIVE button

        // Active button at bottom
        activeButton.setBounds (bounds.removeFromBottom (26));
    }

private:
    //==============================================================================
    void updateBypassState()
    {
        bool isBypassed = activeButton.getToggleState();
        bool isActive = !isBypassed;

        // Update button label
        activeButton.setButtonText (isActive ? "ACTIVE" : "INACTIVE");

        // Enable/disable controls
        hpfKnob.setEnabled (isActive);
        lpfKnob.setEnabled (isActive);
        hpfSlopeButton.setEnabled (isActive);
        hpfQButton.setEnabled (isActive);
        lpfSlopeButton.setEnabled (isActive);
        lpfQButton.setEnabled (isActive);
        postButton.setEnabled (isActive);
        hpfLabel.setAlpha (isActive ? 1.0f : 0.4f);
        lpfLabel.setAlpha (isActive ? 1.0f : 0.4f);
        sectionLabel.setAlpha (isActive ? 1.0f : 0.4f);

        repaint();
    }

    void updateSlopeButtonLabels()
    {
        // Update labels based on current parameter values
        auto hpfSlopeValue = apvtsRef.getRawParameterValue ("hpfSlope");
        if (hpfSlopeValue != nullptr)
            hpfSlopeButton.setButtonText (*hpfSlopeValue > 0.5f ? "18 dB/oct" : "12 dB/oct");

        auto lpfSlopeValue = apvtsRef.getRawParameterValue ("lpfSlope");
        if (lpfSlopeValue != nullptr)
            // FIXED: parameter OFF (<=0.5) = 6 dB/oct, parameter ON (>0.5) = 12 dB/oct
            lpfSlopeButton.setButtonText (*lpfSlopeValue > 0.5f ? "12 dB/oct" : "6 dB/oct");
    }

    //==============================================================================
    juce::AudioProcessorValueTreeState& apvtsRef;

    juce::Label hpfLabel, lpfLabel;
    juce::Slider hpfKnob, lpfKnob;
    juce::ToggleButton hpfSlopeButton, hpfQButton;
    juce::ToggleButton lpfSlopeButton, lpfQButton;
    juce::ToggleButton postButton;
    juce::ToggleButton activeButton;
    juce::Label sectionLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hpfAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpfAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hpfSlopeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hpfQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lpfSlopeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lpfQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> postAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FiltersSectionComponent)
};
