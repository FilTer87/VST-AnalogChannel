/*
  ==============================================================================

    ConsoleSectionComponent.h
    GUI component for Console section
    - Algorithm selector (Clean, Pure, Oxford, Essex, USA)
    - Drive knob
    - Bypass button

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"
#include "AnalogChannelLookAndFeel.h"

class ConsoleSectionComponent : public juce::Component,
                                 private juce::Button::Listener,
                                 private juce::ComboBox::Listener
{
public:
    ConsoleSectionComponent (juce::AudioProcessorValueTreeState& apvts)
        : apvtsRef (apvts),
          driveLAF (juce::Colour (0xff1a1a1a))  // Default dark/black
    {
        // Apply dynamic colored LookAndFeel to drive knob
        driveKnob.setLookAndFeel (&driveLAF);

        // Algorithm selector
        algorithmCombo.addItem ("Clean", 1);
        algorithmCombo.addItem ("Pure", 2);
        algorithmCombo.addItem ("Oxford", 3);
        algorithmCombo.addItem ("Essex", 4);
        algorithmCombo.addItem ("USA", 5);
        algorithmCombo.addListener (this);  // Listen for changes
        addAndMakeVisible (algorithmCombo);

        algorithmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            apvts, "consoleAlgo", algorithmCombo);

        // Update initial color based on current selection
        updateKnobColor();

        // Drive knob
        driveKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        driveKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
        driveKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (driveKnob);

        driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "consoleDrive", driveKnob);

        // Drive label
        driveLabel.setText ("DRIVE", juce::dontSendNotification);
        driveLabel.setJustificationType (juce::Justification::centred);
        driveLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (driveLabel);

        // Active/Inactive button (inverted bypass logic)
        activeButton.setButtonText ("ACTIVE");
        activeButton.setClickingTogglesState (true);
        activeButton.addListener (this);
        addAndMakeVisible (activeButton);

        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "consoleBypass", activeButton);

        // Section label
        sectionLabel.setText ("CONSOLE", juce::dontSendNotification);
        sectionLabel.setJustificationType (juce::Justification::centred);
        sectionLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        sectionLabel.setFont (juce::Font (11.0f, juce::Font::bold));
        addAndMakeVisible (sectionLabel);

        // Initialize state
        updateBypassState();
    }

    ~ConsoleSectionComponent() override
    {
        driveKnob.setLookAndFeel (nullptr);
        algorithmCombo.removeListener (this);
    }

    //==============================================================================
    void comboBoxChanged (juce::ComboBox* comboBox) override
    {
        if (comboBox == &algorithmCombo)
        {
            updateKnobColor();
        }
    }

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

        // Background panel with custom color (#6f684f)
        g.setColour (juce::Colour (0xff6f684f));
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
        sectionLabel.setBounds (bounds.removeFromTop (22));
        bounds.removeFromTop (4);

        // Algorithm selector (full width)
        algorithmCombo.setBounds (bounds.removeFromTop (24));
        bounds.removeFromTop (8);

        // Drive knob
        driveLabel.setBounds (bounds.removeFromTop (15));
        driveKnob.setBounds (bounds.removeFromTop (80));
        bounds.removeFromTop (10);

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
        algorithmCombo.setEnabled (isActive);
        driveKnob.setEnabled (isActive);
        driveLabel.setAlpha (isActive ? 1.0f : 0.4f);
        sectionLabel.setAlpha (isActive ? 1.0f : 0.4f);

        repaint();
    }

    void updateKnobColor()
    {
        // Console: Clean=dark, Pure=purple, Oxford=orange, Essex=blue, USA=red
        int selectedId = algorithmCombo.getSelectedId();
        juce::Colour newColor;

        switch (selectedId)
        {
            case 1: newColor = juce::Colour (0xff1a1a1a); break; // Clean = dark
            case 2: newColor = juce::Colour (0xff9370DB); break; // Pure = purple
            case 3: newColor = juce::Colour (0xffFFA500); break; // Oxford = orange
            case 4: newColor = juce::Colour (0xff4169E1); break; // Essex = blue
            case 5: newColor = juce::Colour (0xffDC143C); break; // USA = red (crimson)
            default: newColor = juce::Colour (0xff1a1a1a); break;
        }

        driveLAF.setCenterColor (newColor);
        driveKnob.repaint();
    }

    //==============================================================================
    juce::AudioProcessorValueTreeState& apvtsRef;

    juce::ComboBox algorithmCombo;
    juce::Slider driveKnob;
    juce::Label driveLabel;
    juce::ToggleButton activeButton;
    juce::Label sectionLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algorithmAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    // Dynamic colored LookAndFeel for drive knob
    DynamicColoredKnobLookAndFeel driveLAF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleSectionComponent)
};
