/*
  ==============================================================================

    OutStageSectionComponent.h
    GUI component for OutStage section (120px column)
    - Algorithm selector (Clean, Hard Clip, Soft Clip)
    - Drive knob
    - GR indicator LED

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"
#include "AnalogChannelLookAndFeel.h"

class OutStageSectionComponent : public juce::Component,
                                   private juce::Button::Listener,
                                   private juce::ComboBox::Listener
{
public:
    OutStageSectionComponent (juce::AudioProcessorValueTreeState& apvts)
        : apvtsRef (apvts),
          driveLAF (juce::Colour (0xff1a1a1a))
    {
        driveKnob.setLookAndFeel (&driveLAF);

        // Algorithm selector
        algorithmCombo.addItem ("Clean", 1);
        algorithmCombo.addItem ("Pure", 2);
        algorithmCombo.addItem ("Tape", 3);
        algorithmCombo.addItem ("Tube", 4);
        algorithmCombo.addItem ("Hard Clip", 5);
        algorithmCombo.addItem ("Soft Clip", 6);
        algorithmCombo.addListener (this);
        addAndMakeVisible (algorithmCombo);

        algorithmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            apvts, "outStageAlgo", algorithmCombo);

        updateKnobColor();

        // Drive knob
        driveKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        driveKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
        driveKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (driveKnob);

        driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "outStageDrive", driveKnob);

        // Drive label
        driveLabel.setText ("DRIVE", juce::dontSendNotification);
        driveLabel.setJustificationType (juce::Justification::centred);
        driveLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (driveLabel);

        // GR indicator LED (simple label, color updated by parent)
        grIndicator.setText ("GR", juce::dontSendNotification);
        grIndicator.setJustificationType (juce::Justification::centred);
        grIndicator.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        grIndicator.setColour (juce::Label::backgroundColourId, AnalogChannelColors::LED_OFF);
        addAndMakeVisible (grIndicator);

        // Active/Inactive button (inverted bypass logic)
        activeButton.setButtonText ("ACTIVE");
        activeButton.setClickingTogglesState (true);
        activeButton.addListener (this);
        addAndMakeVisible (activeButton);

        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "outStageBypass", activeButton);

        // Section label
        sectionLabel.setText ("OUT STAGE", juce::dontSendNotification);
        sectionLabel.setJustificationType (juce::Justification::centred);
        sectionLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        sectionLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        addAndMakeVisible (sectionLabel);

        // Initialize state
        updateBypassState();
    }

    ~OutStageSectionComponent() override
    {
        driveKnob.setLookAndFeel (nullptr);
        algorithmCombo.removeListener (this);
    }

    //==============================================================================
    void comboBoxChanged (juce::ComboBox* comboBox) override
    {
        if (comboBox == &algorithmCombo)
            updateKnobColor();
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
        sectionLabel.setBounds (bounds.removeFromTop (22));
        bounds.removeFromTop (4);

        // Algorithm selector (full width)
        algorithmCombo.setBounds (bounds.removeFromTop (24));
        bounds.removeFromTop (8);

        // Drive knob
        driveLabel.setBounds (bounds.removeFromTop (15));
        driveKnob.setBounds (bounds.removeFromTop (80));
        bounds.removeFromTop (10);

        // GR indicator
        grIndicator.setBounds (bounds.removeFromTop (30).reduced (20, 0));
        bounds.removeFromTop (10);

        // Active button at bottom
        activeButton.setBounds (bounds.removeFromBottom (26));
    }

    //==============================================================================
    // Accessors for parent to update GR indicator
    juce::Label& getGRIndicatorLeft() { return grIndicator; }

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
        grIndicator.setAlpha (isActive ? 1.0f : 0.4f);
        sectionLabel.setAlpha (isActive ? 1.0f : 0.4f);

        repaint();
    }

    void updateKnobColor()
    {
        // OutStage: Clean=dark, Pure=purple, Tape=yellow, Tube=blue, Soft Clip=brown, Hard Clip=red
        int selectedId = algorithmCombo.getSelectedId();
        juce::Colour newColor;

        switch (selectedId)
        {
            case 1: newColor = juce::Colour (0xff1a1a1a); break; // Clean = dark
            case 2: newColor = juce::Colour (0xff9370DB); break; // Pure = purple
            case 3: newColor = juce::Colour (0xffFFD700); break; // Tape = gold/yellow
            case 4: newColor = juce::Colour (0xff4169E1); break; // Tube = blue
            case 5: newColor = juce::Colour (0xffDC143C); break; // Hard Clip = crimson red
            case 6: newColor = juce::Colour (0xff8B4513); break; // Soft Clip = saddle brown
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
    juce::Label grIndicator;
    juce::ToggleButton activeButton;
    juce::Label sectionLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algorithmAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    DynamicColoredKnobLookAndFeel driveLAF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutStageSectionComponent)
};
