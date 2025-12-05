/*
  ==============================================================================

    PluginHeaderBar.h
    Reusable header bar component for all KuramaSound plugins

    Features:
    - KuramaSound logo (favicon)
    - Plugin name (bold) + "| KuramaSound" (normal)
    - Menu button (☰) with customizable menu items
    - Fixed 28px height, consistent styling

    Copyright (c) 2025 KuramaSound

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <functional>

//==============================================================================
/**
    Reusable header bar component for KuramaSound plugins

    Usage example:

    // In PluginEditor constructor:
    headerBar.setPluginName ("MixStrip");
    headerBar.setMenuCallback ([this] (juce::PopupMenu& menu) {
        menu.addItem (1, "User Manual");
        menu.addItem (2, "About");
    });
    headerBar.setMenuResultCallback ([this] (int result) {
        if (result == 1) openUserManual();
        else if (result == 2) showAboutDialog();
    });
    addAndMakeVisible (headerBar);
*/
class PluginHeaderBar : public juce::Component
{
public:
    //==============================================================================
    PluginHeaderBar();
    ~PluginHeaderBar() override = default;

    //==============================================================================
    // Configuration

    /** Set the plugin name (e.g., "AnalogChannel", "MixStrip") */
    void setPluginName (const juce::String& name);

    /**
     * Set callback to populate the menu.
     * Called when user clicks the menu button.
     * Add your menu items inside this callback.
     *
     * Example:
     * headerBar.setMenuCallback ([this] (juce::PopupMenu& menu) {
     *     menu.addItem (1, "User Manual");
     *     menu.addItem (2, "About");
     *     menu.addSeparator();
     *     menu.addItem (3, "Settings");
     * });
     */
    void setMenuCallback (std::function<void(juce::PopupMenu&)> callback);

    /**
     * Set callback to handle menu item selection.
     * Called when user selects a menu item.
     *
     * Example:
     * headerBar.setMenuResultCallback ([this] (int result) {
     *     switch (result) {
     *         case 1: openUserManual(); break;
     *         case 2: showAboutDialog(); break;
     *         case 3: openSettings(); break;
     *     }
     * });
     */
    void setMenuResultCallback (std::function<void(int)> callback);

    //==============================================================================
    // Component overrides
    void paint (juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Constants
    static constexpr int HEIGHT = 28;  // Fixed header height

private:
    //==============================================================================
    void showMenu();

    //==============================================================================
    juce::String pluginName = "Plugin";  // Default name
    juce::Image faviconImage;
    juce::TextButton menuButton;

    // Callbacks
    std::function<void(juce::PopupMenu&)> onMenuPopulate;
    std::function<void(int)> onMenuResult;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginHeaderBar)
};
