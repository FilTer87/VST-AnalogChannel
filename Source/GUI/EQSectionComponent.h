/*
  ==============================================================================

    EQSectionComponent.h
    GUI component for EQ section (250px column)
    - Bass shelf (gain only)
    - Bell 1 (frequency selector + gain)
    - Bell 2 (frequency selector + gain)
    - Treble shelf (gain only)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"
#include "AnalogChannelLookAndFeel.h"

class EQSectionComponent : public juce::Component,
                           private juce::Slider::Listener,
                           private juce::Button::Listener
{
public:
    EQSectionComponent (juce::AudioProcessorValueTreeState& apvts)
        : apvtsRef (apvts),
          // Initialize colored LookAndFeels with specific colors for each filter
          trebleLAF (juce::Colour (0xff87CEEB)),      // Sky blue (celeste) for Treble
          trebleFreqLAF (juce::Colour (0xff87CEEB)),  // Sky blue for Treble freq
          bassLAF (juce::Colour (0xff4169E1)),        // Royal blue for Bass
          bassFreqLAF (juce::Colour (0xff4169E1)),    // Royal blue for Bass freq
          bell1LAF (juce::Colour (0xff32CD32)),       // Lime green for Bell 1
          bell1FreqLAF (juce::Colour (0xff32CD32)),   // Lime green for Bell 1 freq
          bell2LAF (juce::Colour (0xffFFA500)),       // Orange for Bell 2
          bell2FreqLAF (juce::Colour (0xffFFA500))    // Orange for Bell 2 freq
    {
        // Apply colored LookAndFeel to each knob
        trebleKnob.setLookAndFeel (&trebleLAF);
        trebleFreqKnob.setLookAndFeel (&trebleFreqLAF);
        bassKnob.setLookAndFeel (&bassLAF);
        bassFreqKnob.setLookAndFeel (&bassFreqLAF);
        bell1GainKnob.setLookAndFeel (&bell1LAF);
        bell1FreqKnob.setLookAndFeel (&bell1FreqLAF);
        bell2GainKnob.setLookAndFeel (&bell2LAF);
        bell2FreqKnob.setLookAndFeel (&bell2FreqLAF);
        // Bass gain knob
        bassKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        bassKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 45, 18);
        bassKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (bassKnob);

        bassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "eqBass", bassKnob);

        bassLabel.setText ("BASS", juce::dontSendNotification);
        bassLabel.setJustificationType (juce::Justification::centred);
        bassLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (bassLabel);

        // Bass frequency knob (small, continuous)
        bassFreqKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        bassFreqKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (bassFreqKnob);

        bassFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "eqBassFreq", bassFreqKnob);

        // Bass frequency label (shows 0-10 scale, inverted from frequency)
        bassFreqLabel.setText ("0.0", juce::dontSendNotification);
        bassFreqLabel.setJustificationType (juce::Justification::centred);
        bassFreqLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        bassFreqLabel.setFont (juce::FontOptions (8.0f));
        addAndMakeVisible (bassFreqLabel);

        // Bass frequency knob label
        bassMidCutLabel.setText ("Mid Cut", juce::dontSendNotification);
        bassMidCutLabel.setJustificationType (juce::Justification::centred);
        bassMidCutLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        bassMidCutLabel.setFont (juce::FontOptions (7.0f));
        addAndMakeVisible (bassMidCutLabel);

        bassFreqKnob.addListener (this);

        // Bell 1 frequency knob (stepped, 10 positions)
        bell1FreqKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        bell1FreqKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (bell1FreqKnob);

        bell1FreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "eqBell1Freq", bell1FreqKnob);

        // Bell 1 frequency label (shows selected frequency)
        bell1FreqLabel.setText ("1k", juce::dontSendNotification);
        bell1FreqLabel.setJustificationType (juce::Justification::centred);
        bell1FreqLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        bell1FreqLabel.setFont (juce::FontOptions (9.0f));
        addAndMakeVisible (bell1FreqLabel);

        // Add listener to update label
        bell1FreqKnob.addListener (this);

        bell1GainKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        bell1GainKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 45, 18);
        bell1GainKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (bell1GainKnob);

        bell1GainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "eqBell1Gain", bell1GainKnob);

        bell1Label.setText ("BELL 1", juce::dontSendNotification);
        bell1Label.setJustificationType (juce::Justification::centred);
        bell1Label.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (bell1Label);

        // Bell 2 frequency knob (stepped, 10 positions)
        bell2FreqKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        bell2FreqKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (bell2FreqKnob);

        bell2FreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "eqBell2Freq", bell2FreqKnob);

        // Bell 2 frequency label (shows selected frequency)
        bell2FreqLabel.setText ("3.5k", juce::dontSendNotification);
        bell2FreqLabel.setJustificationType (juce::Justification::centred);
        bell2FreqLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        bell2FreqLabel.setFont (juce::FontOptions (9.0f));
        addAndMakeVisible (bell2FreqLabel);

        // Add listener to update label
        bell2FreqKnob.addListener (this);

        bell2GainKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        bell2GainKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 45, 18);
        bell2GainKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (bell2GainKnob);

        bell2GainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "eqBell2Gain", bell2GainKnob);

        bell2Label.setText ("BELL 2", juce::dontSendNotification);
        bell2Label.setJustificationType (juce::Justification::centred);
        bell2Label.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (bell2Label);

        // Treble gain knob
        trebleKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        trebleKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 45, 18);
        trebleKnob.setTextValueSuffix (" dB");
        addAndMakeVisible (trebleKnob);

        trebleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "eqTreble", trebleKnob);

        trebleLabel.setText ("TREBLE", juce::dontSendNotification);
        trebleLabel.setJustificationType (juce::Justification::centred);
        trebleLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        addAndMakeVisible (trebleLabel);

        // Treble frequency knob (small, continuous)
        trebleFreqKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        trebleFreqKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (trebleFreqKnob);

        trebleFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, "eqTrebleFreq", trebleFreqKnob);

        // Treble frequency label (shows 0-10 scale)
        trebleFreqLabel.setText ("0.0", juce::dontSendNotification);
        trebleFreqLabel.setJustificationType (juce::Justification::centred);
        trebleFreqLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        trebleFreqLabel.setFont (juce::FontOptions (8.0f));
        addAndMakeVisible (trebleFreqLabel);

        // Treble frequency knob label
        trebleMidCutLabel.setText ("Mid Cut", juce::dontSendNotification);
        trebleMidCutLabel.setJustificationType (juce::Justification::centred);
        trebleMidCutLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        trebleMidCutLabel.setFont (juce::FontOptions (7.0f));
        addAndMakeVisible (trebleMidCutLabel);

        trebleFreqKnob.addListener (this);

        // Active/Inactive button (inverted bypass logic)
        activeButton.setButtonText ("ACTIVE");
        activeButton.setClickingTogglesState (true);
        activeButton.addListener (this);
        addAndMakeVisible (activeButton);

        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, "eqBypass", activeButton);

        // Section label
        sectionLabel.setText ("EQUALIZER", juce::dontSendNotification);
        sectionLabel.setJustificationType (juce::Justification::centred);
        sectionLabel.setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_HIGHLIGHT);
        sectionLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        addAndMakeVisible (sectionLabel);

        // Initialize frequency labels based on current parameter values
        updateFrequencyLabels();

        // Initialize state
        updateBypassState();
    }

    ~EQSectionComponent() override
    {
        // Reset all LookAndFeel before destruction
        trebleKnob.setLookAndFeel (nullptr);
        trebleFreqKnob.setLookAndFeel (nullptr);
        bassKnob.setLookAndFeel (nullptr);
        bassFreqKnob.setLookAndFeel (nullptr);
        bell1GainKnob.setLookAndFeel (nullptr);
        bell1FreqKnob.setLookAndFeel (nullptr);
        bell2GainKnob.setLookAndFeel (nullptr);
        bell2FreqKnob.setLookAndFeel (nullptr);
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

    //==============================================================================
    void sliderValueChanged (juce::Slider* slider) override
    {
        if (slider == &bell1FreqKnob)
        {
            bell1FreqLabel.setText (getFrequencyString (static_cast<int> (bell1FreqKnob.getValue())),
                                    juce::dontSendNotification);
        }
        else if (slider == &bell2FreqKnob)
        {
            bell2FreqLabel.setText (getFrequencyString (static_cast<int> (bell2FreqKnob.getValue())),
                                    juce::dontSendNotification);
        }
        else if (slider == &bassFreqKnob)
        {
            // Bass: inverted scale (6500Hz = 0.0, 600Hz = 10.0)
            float freqHz = bassFreqKnob.getValue();
            float midCutValue = juce::jmap (freqHz, 6500.0f, 600.0f, 0.0f, 10.0f);
            bassFreqLabel.setText (juce::String (midCutValue, 1), juce::dontSendNotification);
        }
        else if (slider == &trebleFreqKnob)
        {
            // Treble: direct scale (3500Hz = 0.0, 8200Hz = 10.0)
            float freqHz = trebleFreqKnob.getValue();
            float midCutValue = juce::jmap (freqHz, 3500.0f, 8200.0f, 0.0f, 10.0f);
            trebleFreqLabel.setText (juce::String (midCutValue, 1), juce::dontSendNotification);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (8);
        const int freqKnobSize = 35;   // Small knob for frequency
        const int gainKnobSize = 70;   // Larger for gain adjustment
        const int bellFreqKnobSize = 45; // Bell frequency knobs (existing)
        const int spacing = 4;
        const int horizontalSpacing = 4;

        // Section label at top
        sectionLabel.setBounds (bounds.removeFromTop (22));
        bounds.removeFromTop (4);

        // Treble (now at top) - with frequency knob on the side
        trebleLabel.setBounds (bounds.removeFromTop (14));
        auto trebleRow = bounds.removeFromTop (gainKnobSize);
        auto trebleFreqArea = trebleRow.removeFromRight (freqKnobSize);
        trebleRow.removeFromRight (horizontalSpacing);
        trebleKnob.setBounds (trebleRow);
        // Position freq knob and labels vertically in the area
        trebleMidCutLabel.setBounds (trebleFreqArea.removeFromTop (10));
        trebleFreqKnob.setBounds (trebleFreqArea.removeFromTop (freqKnobSize));
        trebleFreqLabel.setBounds (trebleFreqArea.removeFromTop (12));
        bounds.removeFromTop (spacing);

        // Bell 1
        bell1Label.setBounds (bounds.removeFromTop (14));
        bell1FreqKnob.setBounds (bounds.removeFromTop (bellFreqKnobSize));
        bell1FreqLabel.setBounds (bounds.removeFromTop (12));
        bell1GainKnob.setBounds (bounds.removeFromTop (gainKnobSize));
        bounds.removeFromTop (spacing);

        // Bell 2
        bell2Label.setBounds (bounds.removeFromTop (14));
        bell2FreqKnob.setBounds (bounds.removeFromTop (bellFreqKnobSize));
        bell2FreqLabel.setBounds (bounds.removeFromTop (12));
        bell2GainKnob.setBounds (bounds.removeFromTop (gainKnobSize));
        bounds.removeFromTop (spacing);

        // Bass (now at bottom, before bypass button) - with frequency knob on the side
        bassLabel.setBounds (bounds.removeFromTop (14));
        auto bassRow = bounds.removeFromTop (gainKnobSize);
        auto bassFreqArea = bassRow.removeFromRight (freqKnobSize);
        bassRow.removeFromRight (horizontalSpacing);
        bassKnob.setBounds (bassRow);
        // Position freq knob and labels vertically in the area
        bassMidCutLabel.setBounds (bassFreqArea.removeFromTop (10));
        bassFreqKnob.setBounds (bassFreqArea.removeFromTop (freqKnobSize));
        bassFreqLabel.setBounds (bassFreqArea.removeFromTop (12));
        bounds.removeFromTop (8);

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
        bassKnob.setEnabled (isActive);
        bassFreqKnob.setEnabled (isActive);
        trebleKnob.setEnabled (isActive);
        trebleFreqKnob.setEnabled (isActive);
        bell1FreqKnob.setEnabled (isActive);
        bell2FreqKnob.setEnabled (isActive);
        bell1GainKnob.setEnabled (isActive);
        bell2GainKnob.setEnabled (isActive);
        bassLabel.setAlpha (isActive ? 1.0f : 0.4f);
        bassFreqLabel.setAlpha (isActive ? 1.0f : 0.4f);
        bassMidCutLabel.setAlpha (isActive ? 1.0f : 0.4f);
        trebleLabel.setAlpha (isActive ? 1.0f : 0.4f);
        trebleFreqLabel.setAlpha (isActive ? 1.0f : 0.4f);
        trebleMidCutLabel.setAlpha (isActive ? 1.0f : 0.4f);
        bell1Label.setAlpha (isActive ? 1.0f : 0.4f);
        bell2Label.setAlpha (isActive ? 1.0f : 0.4f);
        bell1FreqLabel.setAlpha (isActive ? 1.0f : 0.4f);
        bell2FreqLabel.setAlpha (isActive ? 1.0f : 0.4f);
        sectionLabel.setAlpha (isActive ? 1.0f : 0.4f);

        repaint();
    }

    void updateFrequencyLabels()
    {
        // Update Bell 1 frequency label from parameter
        bell1FreqLabel.setText (getFrequencyString (static_cast<int> (bell1FreqKnob.getValue())),
                                juce::dontSendNotification);

        // Update Bell 2 frequency label from parameter
        bell2FreqLabel.setText (getFrequencyString (static_cast<int> (bell2FreqKnob.getValue())),
                                juce::dontSendNotification);

        // Update Bass frequency label from parameter (inverted scale)
        float bassFreqHz = bassFreqKnob.getValue();
        float bassMidCutValue = juce::jmap (bassFreqHz, 6500.0f, 600.0f, 0.0f, 10.0f);
        bassFreqLabel.setText (juce::String (bassMidCutValue, 1), juce::dontSendNotification);

        // Update Treble frequency label from parameter (direct scale)
        float trebleFreqHz = trebleFreqKnob.getValue();
        float trebleMidCutValue = juce::jmap (trebleFreqHz, 3500.0f, 8200.0f, 0.0f, 10.0f);
        trebleFreqLabel.setText (juce::String (trebleMidCutValue, 1), juce::dontSendNotification);
    }

    // Helper function to convert parameter index to frequency string (for Bell filters)
    juce::String getFrequencyString (int index)
    {
        const juce::StringArray freqs = { "50", "100", "200", "300", "400", "500", "700", "900", "1.4k", "2.4k", "3.5k", "5k", "7.5k", "10k", "13k" };
        if (index >= 0 && index < freqs.size())
            return freqs[index];
        return "1k";
    }

    // Helper function to format continuous frequency value in Hz
    juce::String formatFrequencyHz (float freqHz)
    {
        if (freqHz >= 1000.0f)
        {
            float freqKHz = freqHz / 1000.0f;
            // Format with 1 decimal if needed
            if (freqKHz >= 10.0f)
                return juce::String (static_cast<int> (freqKHz + 0.5f)) + "k";
            else
                return juce::String (freqKHz, 1) + "k";
        }
        else
        {
            return juce::String (static_cast<int> (freqHz + 0.5f));
        }
    }

    //==============================================================================
    juce::AudioProcessorValueTreeState& apvtsRef;

    juce::Slider bassKnob, bassFreqKnob;
    juce::Slider trebleKnob, trebleFreqKnob;
    juce::Slider bell1FreqKnob, bell2FreqKnob;
    juce::Slider bell1GainKnob, bell2GainKnob;
    juce::Label bassFreqLabel, trebleFreqLabel;
    juce::Label bassMidCutLabel, trebleMidCutLabel;  // "Mid Cut" labels
    juce::Label bell1FreqLabel, bell2FreqLabel;
    juce::Label bassLabel, bell1Label, bell2Label, trebleLabel;
    juce::ToggleButton activeButton;
    juce::Label sectionLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bell1FreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bell1GainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bell2FreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bell2GainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    // Color-coded LookAndFeel instances for each EQ filter
    ColoredKnobLookAndFeel trebleLAF;           // Sky blue for Treble gain
    ColoredPointerKnobLookAndFeel trebleFreqLAF; // Sky blue for Treble freq
    ColoredKnobLookAndFeel bassLAF;             // Royal blue for Bass gain
    ColoredPointerKnobLookAndFeel bassFreqLAF;   // Royal blue for Bass freq
    ColoredKnobLookAndFeel bell1LAF;            // Purple for Bell 1 gain
    ColoredPointerKnobLookAndFeel bell1FreqLAF;  // Purple for Bell 1 freq
    ColoredKnobLookAndFeel bell2LAF;            // Orange for Bell 2 gain
    ColoredPointerKnobLookAndFeel bell2FreqLAF;  // Orange for Bell 2 freq

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQSectionComponent)
};
