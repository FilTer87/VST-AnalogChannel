/*
  ==============================================================================

    PluginHeaderBar.cpp
    Reusable header bar component for all KuramaSound plugins

    Copyright (c) 2025 KuramaSound

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

  ==============================================================================
*/

#include "PluginHeaderBar.h"
#include "KuramaColors.h"

//==============================================================================
PluginHeaderBar::PluginHeaderBar()
{
    // Load KuramaSound favicon from binary resources
    faviconImage = juce::ImageCache::getFromMemory (BinaryData::favicon32x32_png,
                                                     BinaryData::favicon32x32_pngSize);

    // Setup menu button with ☰ icon
    menuButton.setButtonText (juce::CharPointer_UTF8 ("\xe2\x98\xb0"));  // UTF-8 for ☰
    menuButton.onClick = [this] { showMenu(); };
    addAndMakeVisible (menuButton);
}

//==============================================================================
void PluginHeaderBar::setPluginName (const juce::String& name)
{
    pluginName = name;
    repaint();
}

void PluginHeaderBar::setMenuCallback (std::function<void(juce::PopupMenu&)> callback)
{
    onMenuPopulate = callback;
}

void PluginHeaderBar::setMenuResultCallback (std::function<void(int)> callback)
{
    onMenuResult = callback;
}

//==============================================================================
void PluginHeaderBar::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.setColour (KuramaColors::HEADER_BG);
    g.fillRect (bounds);

    // Draw favicon (28x28) at left
    if (faviconImage.isValid())
    {
        int faviconSize = 28;
        int faviconX = 6;
        int faviconY = 0;
        auto faviconBounds = juce::Rectangle<int> (faviconX, faviconY, faviconSize, faviconSize);
        g.drawImage (faviconImage, faviconBounds.toFloat(), juce::RectanglePlacement::centred);
    }

    // Draw plugin name with two styles: "[PluginName]" (bold) + " | KuramaSound" (normal)
    int textStartX = 6 + 28 + 8;  // favicon margin + size + spacing
    int yCenter = bounds.getCentreY();

    // First part: plugin name (bold)
    g.setColour (KuramaColors::HEADER_TEXT_BOLD);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    juce::String firstPart = pluginName + " ";
    int firstPartWidth = g.getCurrentFont().getStringWidth (firstPart);
    g.drawText (firstPart, textStartX, yCenter - 7, firstPartWidth, 14,
                juce::Justification::centredLeft);

    // Second part: "| KuramaSound" (normal weight)
    g.setColour (KuramaColors::HEADER_TEXT);
    g.setFont (juce::FontOptions (14.0f, juce::Font::plain));
    g.drawText ("| KuramaSound", textStartX + firstPartWidth, yCenter - 7, 200, 14,
                juce::Justification::centredLeft);

    // Draw borders
    g.setColour (KuramaColors::HEADER_BORDER);
    g.drawLine (0, HEIGHT, getWidth(), HEIGHT, 1.0f);  // Bottom border
    g.drawLine (0, 0, getWidth(), 0, 1.0f);             // Top border
}

void PluginHeaderBar::resized()
{
    auto bounds = getLocalBounds();

    // Menu button: 24x24px with 2px margins (top, right)
    int buttonSize = 24;
    int topMargin = 2;
    int rightMargin = 2;
    int buttonX = bounds.getWidth() - buttonSize - rightMargin;
    int buttonY = topMargin;
    menuButton.setBounds (buttonX, buttonY, buttonSize, buttonSize);
}

//==============================================================================
void PluginHeaderBar::showMenu()
{
    juce::PopupMenu menu;

    // Call user-defined callback to populate menu
    if (onMenuPopulate)
        onMenuPopulate (menu);

    // Show menu and handle result
    menu.showMenuAsync (juce::PopupMenu::Options(),
                        [this] (int result)
                        {
                            if (result != 0 && onMenuResult)
                                onMenuResult (result);
                        });
}
