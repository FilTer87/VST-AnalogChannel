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
      lowDynamicSection (p.getValueTreeState()),
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
    addAndMakeVisible (lowDynamicSection);
    addAndMakeVisible (eqSection);
    addAndMakeVisible (styleCompSection);
    addAndMakeVisible (consoleSection);
    addAndMakeVisible (outStageSection);
    addAndMakeVisible (analogChannelsSection);
    addAndMakeVisible (volumeSection);

    // Setup menu button (☰ icon in header)
    menuButton.setButtonText (juce::CharPointer_UTF8 ("\xe2\x98\xb0"));  // UTF-8 for ☰
    menuButton.onClick = [this] { showOptionsMenu(); };
    addAndMakeVisible (menuButton);

    // Load saved zoom preference and register listener
    audioProcessor.getValueTreeState().addParameterListener ("guiZoom", this);

    auto* zoomParam = dynamic_cast<juce::AudioParameterChoice*> (
        audioProcessor.getValueTreeState().getParameter ("guiZoom"));

    if (zoomParam != nullptr)
    {
        int zoomIndex = zoomParam->getIndex();  // Get choice index directly (0-3)
        const float zoomScales[] = { 0.75f, 1.0f, 1.25f, 1.5f };
        applyZoomScale (zoomScales[zoomIndex]);
    }
    else
    {
        // Default size if parameter not found (fallback to 125%)
        applyZoomScale (1.25f);
    }

    // Start timer for meter updates (30 Hz)
    startTimerHz (30);
}

AnalogChannelAudioProcessorEditor::~AnalogChannelAudioProcessorEditor()
{
    audioProcessor.getValueTreeState().removeParameterListener ("guiZoom", this);
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void AnalogChannelAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background
    g.fillAll (AnalogChannelColors::BG_DARK);

    // Title bar area
    auto titleArea = getLocalBounds().removeFromTop (28);
    g.setColour (AnalogChannelColors::PANEL_BG);
    g.fillRect (titleArea);

    // Draw title with two styles: "AnalogChannel" (bold) + " | KuramaSound" (normal)
    auto titleTextArea = titleArea.reduced (8, 0);
    int xPos = titleTextArea.getX();
    int yCenter = titleArea.getCentreY();

    // First part: "AnalogChannel " (bold)
    g.setColour (AnalogChannelColors::TEXT_HIGHLIGHT);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    juce::String firstPart = "AnalogChannel ";
    int firstPartWidth = g.getCurrentFont().getStringWidth (firstPart);
    g.drawText (firstPart, xPos, yCenter - 7, firstPartWidth, 14, juce::Justification::centredLeft);

    // Second part: "| KuramaSound" (normal weight)
    g.setFont (juce::FontOptions (14.0f, juce::Font::plain));
    g.drawText ("| KuramaSound", xPos + firstPartWidth, yCenter - 7, 200, 14, juce::Justification::centredLeft);

    // Draw borders
    g.setColour (AnalogChannelColors::BORDER_LIGHT);
    g.drawLine (0, 28, getWidth(), 28, 1.0f);  // Title bar bottom border
    g.drawLine (0, 0, getWidth(), 0, 1.0f);  // Top border
    g.drawLine (0, getHeight() - 1, getWidth(), getHeight() - 1, 1.0f);  // Bottom border
}

void AnalogChannelAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Add padding (top/bottom margins)
    bounds.removeFromTop (4);     // Top padding
    bounds.removeFromBottom (4);  // Bottom padding

    // Title bar (28px) - position menu button here
    auto titleBar = bounds.removeFromTop (28);

    // Menu button: 24x24px with 2px margins (top, bottom, right)
    // Title bar starts at y=0, height=28px, so button should be at y=2 to center vertically
    int buttonSize = 24;
    int topMargin = 2;
    int rightMargin = 2;
    int buttonX = getWidth() - buttonSize - rightMargin;
    int buttonY = topMargin;  // Start directly from top (title bar starts at 0)
    menuButton.setBounds (buttonX, buttonY, buttonSize, buttonSize);

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

    // Column 2: ControlComp (top 50%) + LowDynamic (bottom 50%)
    auto col2 = mainArea.removeFromLeft (colWidth);
    controlCompSection.setBounds (col2.removeFromTop (col2.getHeight() / 2 - 2));
    col2.removeFromTop (4);  // Spacing
    lowDynamicSection.setBounds (col2);
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

void AnalogChannelAudioProcessorEditor::showOptionsMenu()
{
    juce::PopupMenu menu;

    // User Manual
    menu.addItem (1, "User Manual", true);

    // About / Credits
    menu.addItem (2, "About / Credits", true);

    menu.addSeparator();

    // Plugin Size submenu
    juce::PopupMenu sizeMenu;
    sizeMenu.addItem (10, "75%", true, currentZoomScale == 0.75f);
    sizeMenu.addItem (11, "100%", true, currentZoomScale == 1.0f);
    sizeMenu.addItem (12, "125%", true, currentZoomScale == 1.25f);
    sizeMenu.addItem (13, "150%", true, currentZoomScale == 1.5f);

    menu.addSubMenu ("Plugin Size", sizeMenu);

    // Show menu
    menu.showMenuAsync (juce::PopupMenu::Options(),
                        [this] (int result)
                        {
                            switch (result)
                            {
                                case 1:  // User Manual
                                    juce::URL ("https://github.com/yourusername/AnalogChannel").launchInDefaultBrowser();
                                    break;

                                case 2:  // About / Credits
                                {
                                    juce::AlertWindow::showMessageBoxAsync (
                                        juce::AlertWindow::InfoIcon,
                                        "About AnalogChannel",
                                        "AnalogChannel v1.0\n\n"
                                        "VST3 Channel Strip Plugin\n"
                                        "Copyright (c) 2025 KuramaSound\n\n"
                                        "Algorithms from AirWindows (MIT License)\n"
                                        "Built with JUCE Framework\n\n"
                                        "GPL v3 License",
                                        "OK");
                                    break;
                                }

                                case 10:  // 75%
                                    applyZoomScale (0.75f);
                                    break;

                                case 11:  // 100%
                                    applyZoomScale (1.0f);
                                    break;

                                case 12:  // 125%
                                    applyZoomScale (1.25f);
                                    break;

                                case 13:  // 150%
                                    applyZoomScale (1.5f);
                                    break;

                                default:
                                    break;
                            }
                        });
}

void AnalogChannelAudioProcessorEditor::applyZoomScale (float scale)
{
    currentZoomScale = scale;

    // Base size is 710x580
    const int baseWidth = 710;
    const int baseHeight = 580;

    // IMPORTANT: We use setScaleFactor() which handles both:
    // 1. Scaling the component hierarchy
    // 2. Adjusting the window size automatically
    // This replaces the manual setTransform() + setSize() approach

    // First, reset any previous transform
    setTransform (juce::AffineTransform());

    // Set the base size (unscaled)
    setSize (baseWidth, baseHeight);

    // Apply the scale factor (JUCE handles everything)
    setScaleFactor (scale);

    // Save zoom preference to ValueTreeState
    auto* zoomParam = audioProcessor.getValueTreeState().getParameter ("guiZoom");
    if (zoomParam != nullptr)
    {
        // Map scale to parameter index (0-3)
        int zoomIndex = 2;  // Default: 125%
        if (scale <= 0.75f) zoomIndex = 0;       // 75%
        else if (scale <= 1.0f) zoomIndex = 1;   // 100%
        else if (scale <= 1.25f) zoomIndex = 2;  // 125%
        else zoomIndex = 3;                      // 150%

        zoomParam->setValueNotifyingHost (zoomIndex / 3.0f);  // Normalize to 0-1
    }
}

void AnalogChannelAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float newValue)
{
    // Listen for zoom parameter changes (e.g., when loading presets)
    if (parameterID == "guiZoom")
    {
        auto* zoomParam = dynamic_cast<juce::AudioParameterChoice*> (
            audioProcessor.getValueTreeState().getParameter ("guiZoom"));

        if (zoomParam != nullptr)
        {
            int zoomIndex = zoomParam->getIndex();
            const float zoomScales[] = { 0.75f, 1.0f, 1.25f, 1.5f };

            // Apply the new zoom scale (but don't save it again to avoid loop)
            if (zoomIndex >= 0 && zoomIndex < 4)
            {
                float newScale = zoomScales[zoomIndex];

                // Only apply if different from current scale (avoid unnecessary updates)
                if (std::abs (newScale - currentZoomScale) > 0.01f)
                {
                    currentZoomScale = newScale;

                    // Base size is 710x580
                    const int baseWidth = 710;
                    const int baseHeight = 580;

                    // First, reset any previous transform
                    setTransform (juce::AffineTransform());

                    // Set the base size (unscaled)
                    setSize (baseWidth, baseHeight);

                    // Apply the scale factor (JUCE handles everything)
                    setScaleFactor (newScale);
                }
            }
        }
    }
}
