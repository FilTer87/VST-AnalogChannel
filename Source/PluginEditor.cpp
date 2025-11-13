/*
  ==============================================================================

    PluginEditor.cpp
    AnalogChannel VST3 Channel Strip Plugin - GUI

    Copyright (c) 2025 KuramaSound

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GUI/Colors.h"

//==============================================================================
AnalogChannelAudioProcessorEditor::AnalogChannelAudioProcessorEditor (AnalogChannelAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      preInputSection (p.getValueTreeState()),
      filtersSection (p.getValueTreeState()),
      controlCompSection (p.getValueTreeState()),
      eqSection (p.getValueTreeState()),
      styleCompSection (p.getValueTreeState()),
      consoleSection (p.getValueTreeState()),
      outStageSection (p.getValueTreeState()),
      analogChannelsSection (p.getValueTreeState()),
      volumeSection (p.getValueTreeState())
{
    // Set custom Look & Feel
    setLookAndFeel (&AnalogChannelLAF);

    // Add and make visible: Peak Meters
    addAndMakeVisible (inputMeterLeft);
    addAndMakeVisible (inputMeterRight);
    addAndMakeVisible (outputMeterLeft);
    addAndMakeVisible (outputMeterRight);

    // Add and make visible: Section components
    addAndMakeVisible (preInputSection);
    addAndMakeVisible (filtersSection);
    addAndMakeVisible (controlCompSection);
    addAndMakeVisible (eqSection);
    addAndMakeVisible (styleCompSection);
    addAndMakeVisible (consoleSection);
    addAndMakeVisible (outStageSection);
    addAndMakeVisible (analogChannelsSection);
    addAndMakeVisible (volumeSection);

    // Set editor size (optimized for content + reduced spacing)
    setSize (710, 580);

    // Start timer for meter updates (30 Hz)
    startTimerHz (30);
}

AnalogChannelAudioProcessorEditor::~AnalogChannelAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void AnalogChannelAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background
    g.fillAll (AnalogChannelColors::BG_DARK);

    // Title bar area
    auto titleArea = getLocalBounds().removeFromTop (30);
    g.setColour (AnalogChannelColors::PANEL_BG);
    g.fillRect (titleArea);

    g.setColour (AnalogChannelColors::TEXT_HIGHLIGHT);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("AnalogChannel v1.0", titleArea.reduced (8), juce::Justification::centredLeft);

    // Draw borders
    g.setColour (AnalogChannelColors::BORDER_LIGHT);
    g.drawLine (0, 30, getWidth(), 30, 1.0f);  // Title bar bottom border
    g.drawLine (0, 0, getWidth(), 0, 1.0f);  // Top border
    g.drawLine (0, getHeight() - 1, getWidth(), getHeight() - 1, 1.0f);  // Bottom border
}

void AnalogChannelAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Add padding (top/bottom margins)
    bounds.removeFromTop (4);     // Top padding
    bounds.removeFromBottom (4);  // Bottom padding

    // Title bar (30px)
    bounds.removeFromTop (30);

    // Main area
    auto mainArea = bounds;

    // Input meters (left side, 28px width)
    auto inputMeterArea = mainArea.removeFromLeft (28);
    inputMeterLeft.setBounds (inputMeterArea.removeFromLeft (14));
    inputMeterRight.setBounds (inputMeterArea);

    // Output meters (right side, 28px width)
    auto outputMeterArea = mainArea.removeFromRight (28);
    outputMeterLeft.setBounds (outputMeterArea.removeFromLeft (14));
    outputMeterRight.setBounds (outputMeterArea);

    // Section columns layout (all uniform width)
    const int spacing = 4;
    const int colWidth = 130;  // All columns same width

    // Column 1: PreInput + Filters (split vertically)
    auto col1 = mainArea.removeFromLeft (colWidth);
    preInputSection.setBounds (col1.removeFromTop (col1.getHeight() / 2 - 2));
    col1.removeFromTop (4);  // Spacing
    filtersSection.setBounds (col1);
    mainArea.removeFromLeft (spacing);

    // Column 2: ControlComp (full height)
    auto col2 = mainArea.removeFromLeft (colWidth);
    controlCompSection.setBounds (col2);
    mainArea.removeFromLeft (spacing);

    // Column 3: EQ (full height)
    auto col3 = mainArea.removeFromLeft (colWidth);
    eqSection.setBounds (col3);
    mainArea.removeFromLeft (spacing);

    // Column 4: StyleComp + Console (split vertically: 60% / 40%)
    auto col4 = mainArea.removeFromLeft (colWidth);
    int styleCompHeight = static_cast<int> (col4.getHeight() * 0.60f);
    styleCompSection.setBounds (col4.removeFromTop (styleCompHeight));
    col4.removeFromTop (4);  // Spacing
    consoleSection.setBounds (col4);
    mainArea.removeFromLeft (spacing);

    // Column 5: OutStage + AnalogChannels + Volume (split vertically)
    auto col5 = mainArea.removeFromLeft (colWidth);
    int totalHeight = col5.getHeight();

    // OutStage: 47% of column height
    int outStageHeight = static_cast<int> (totalHeight * 0.50f);
    outStageSection.setBounds (col5.removeFromTop (outStageHeight));
    col5.removeFromTop (4);  // Spacing

    // AnalogChannels: 30% of column height
    int analogChannelsHeight = static_cast<int> (totalHeight * 0.30f);
    analogChannelsSection.setBounds (col5.removeFromTop (analogChannelsHeight));
    col5.removeFromTop (4);  // Spacing

    // Volume: 23% of column height
    volumeSection.setBounds (col5);
}

void AnalogChannelAudioProcessorEditor::timerCallback()
{
    // Update peak meters
    inputMeterLeft.setLevel (audioProcessor.getInputPeakLeft());
    inputMeterRight.setLevel (audioProcessor.getInputPeakRight());
    outputMeterLeft.setLevel (audioProcessor.getOutputPeakLeft());
    outputMeterRight.setLevel (audioProcessor.getOutputPeakRight());

    // Update GR meters in section components
    controlCompSection.getGRMeter().setValue (std::abs (audioProcessor.getControlCompGRLeft()));
    styleCompSection.getGRMeter().setValue (std::abs (audioProcessor.getStyleCompGRLeft()));
    outStageSection.getGRMeter().setValue (std::abs (audioProcessor.getOutStageGRLeft()));
}
