/*
  ==============================================================================

    StyleCompSectionComponent.h
    GUI component for Style-Comp section (140px column)
    - Mode selector (Warm / Punch)
    - Comp IN knob (drive)
    - Makeup knob (smaller)
    - GR LED meter strip

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"
#include "LEDMeterStrip.h"
#include "AnalogChannelLookAndFeel.h"

class StyleCompSectionComponent : public juce::Component,
                                    private juce::Button::Listener,
                                    private juce::ComboBox::Listener
{
public:
    StyleCompSectionComponent (juce::AudioProcessorValueTreeState& apvts)
        : apvtsRef (apvts), grMeter (8),
          compInLAF (juce::Colour (0xff4169E1))  // Default blue for Warm
    {
        compInKnob.setLookAndFeel (&compInLAF);

        // Mode selector
        modeCombo.addItem ("Warm", 1);
        modeCombo.addItem ("Punch", 2);
        modeCombo.addListener (this);
        addAndMakeVisible (modeCombo);

        modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            apvts, "styleCompAlgo", modeCombo);

        updateKnobColor();

        // Comp IN knob
        compInKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        compInKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
        compInKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (compInKnob);

        compInAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "styleCompIn", compInKnob);

        // Comp IN label
        compInLabel.setText ("COMP IN", juce::dontSendNotification);
        compInLabel.setJustificationType (juce::Justification::centred);
        compInLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (compInLabel);

        // Makeup knob (smaller)
        makeupKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        makeupKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 18);
        makeupKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (makeupKnob);

        makeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "styleCompMakeup", makeupKnob);

        // Makeup label
        makeupLabel.setText ("Makeup", juce::dontSendNotification);
        makeupLabel.setJustificationType (juce::Justification::centred);
        makeupLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        makeupLabel.setFont (juce::FontOptions (9.0f));
        addAndMakeVisible (makeupLabel);

        // Mix knob (smaller, same size as Makeup)
        mixKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        mixKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 18);
        mixKnob.setTextValueSuffix (" %");
        addAndMakeVisible (mixKnob);

        mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "styleCompMix", mixKnob);

        // Mix label
        mixLabel.setText ("Mix", juce::dontSendNotification);
        mixLabel.setJustificationType (juce::Justification::centred);
        mixLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        mixLabel.setFont (juce::FontOptions (9.0f));
        addAndMakeVisible (mixLabel);

        // GR meter
        addAndMakeVisible (grMeter);

        // Active/Inactive button (inverted bypass logic)
        activeButton.setButtonText ("ACTIVE");
        activeButton.setClickingTogglesState (true);
        activeButton.addListener (this);
        addAndMakeVisible (activeButton);

        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "styleCompBypass", activeButton);

        // Section label
        sectionLabel.setText ("STYLE-COMP", juce::dontSendNotification);
        sectionLabel.setJustificationType (juce::Justification::centred);
        sectionLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        sectionLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        addAndMakeVisible (sectionLabel);

        // Initialize state
        updateBypassState();
    }

    ~StyleCompSectionComponent() override
    {
        compInKnob.setLookAndFeel (nullptr);
        modeCombo.removeListener (this);
    }

    //==============================================================================
    void comboBoxChanged (juce::ComboBox* comboBox) override
    {
        if (comboBox == &modeCombo)
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
        sectionLabel.setBounds (bounds.removeFromTop (25));
        bounds.removeFromTop (5);

        // Mode selector
        modeCombo.setBounds (bounds.removeFromTop (25));
        bounds.removeFromTop (10);

        // Comp IN knob (large)
        compInLabel.setBounds (bounds.removeFromTop (15));
        compInKnob.setBounds (bounds.removeFromTop (80));
        bounds.removeFromTop (8);

        // Makeup and Mix knobs (smaller, side by side)
        auto makeupMixLabelArea = bounds.removeFromTop (12);
        auto makeupMixKnobArea = bounds.removeFromTop (55);

        // Split area in half
        auto makeupArea = makeupMixKnobArea.removeFromLeft (makeupMixKnobArea.getWidth() / 2);
        auto mixArea = makeupMixKnobArea;

        auto makeupLabelArea = makeupMixLabelArea.removeFromLeft (makeupMixLabelArea.getWidth() / 2);
        auto mixLabelArea = makeupMixLabelArea;

        makeupLabel.setBounds (makeupLabelArea);
        makeupKnob.setBounds (makeupArea);
        mixLabel.setBounds (mixLabelArea);
        mixKnob.setBounds (mixArea);

        bounds.removeFromTop (3);

        // GR meter (no label)
        grMeter.setBounds (bounds.removeFromTop (40).reduced (5, 0));
        bounds.removeFromTop (5);

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
        modeCombo.setEnabled (isActive);
        compInKnob.setEnabled (isActive);
        makeupKnob.setEnabled (isActive);
        mixKnob.setEnabled (isActive);
        compInLabel.setAlpha (isActive ? 1.0f : 0.4f);
        makeupLabel.setAlpha (isActive ? 1.0f : 0.4f);
        mixLabel.setAlpha (isActive ? 1.0f : 0.4f);
        sectionLabel.setAlpha (isActive ? 1.0f : 0.4f);

        repaint();
    }

    void updateKnobColor()
    {
        // StyleComp: Warm=blue, Punch=green
        int selectedId = modeCombo.getSelectedId();
        juce::Colour newColor;

        switch (selectedId)
        {
            case 1: newColor = juce::Colour (0xff4169E1); break; // Warm = blue
            case 2: newColor = juce::Colour (0xff32CD32); break; // Punch = lime green
            default: newColor = juce::Colour (0xff4169E1); break;
        }

        compInLAF.setCenterColor (newColor);
        compInKnob.repaint();
    }

    //==============================================================================
    juce::AudioProcessorValueTreeState& apvtsRef;

    juce::ComboBox modeCombo;
    juce::Slider compInKnob;
    juce::Slider makeupKnob;
    juce::Slider mixKnob;
    juce::Label compInLabel;
    juce::Label makeupLabel;
    juce::Label mixLabel;
    LEDMeterStrip grMeter;
    juce::ToggleButton activeButton;
    juce::Label sectionLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compInAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> makeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    DynamicColoredKnobLookAndFeel compInLAF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StyleCompSectionComponent)
};
