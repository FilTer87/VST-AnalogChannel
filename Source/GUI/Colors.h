/*
  ==============================================================================

    Colors.h
    Color palette for AnalogChannel GUI
    Based on Hardware-style mockup (version B)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace AnalogChannelColors
{
    // ========== Background & Panels ==========
    const juce::Colour BG_DARK       (0xff2a2a2a);  // Dark charcoal background
    const juce::Colour PANEL_BG      (0xff3a3a3a);  // Panel background (lighter)
    const juce::Colour BORDER_DARK   (0xff1a1a1a);  // Dark borders
    const juce::Colour BORDER_LIGHT  (0xff4a4a4a);  // Light borders/highlights

    // ========== Text ==========
    const juce::Colour TEXT_MAIN     (0xffcccccc);  // Main text (silver)
    const juce::Colour TEXT_DIM      (0xff888888);  // Dimmed/secondary text
    const juce::Colour TEXT_HIGHLIGHT(0xffeeeeee);  // Bright text

    // ========== Knobs & Controls ==========
    const juce::Colour KNOB_BODY     (0xff505050);  // Knob metallic gray
    const juce::Colour KNOB_HIGHLIGHT(0xff707070);  // Knob highlight
    const juce::Colour KNOB_SHADOW   (0xff303030);  // Knob shadow (subtle)
    const juce::Colour KNOB_INDICATOR(0xffff6600);  // Orange indicator line

    // ========== Buttons & Toggles ==========
    const juce::Colour BUTTON_OFF    (0xff404040);  // Inactive button
    const juce::Colour BUTTON_ON     (0xffff6600);  // Active button (orange)
    const juce::Colour BUTTON_HOVER  (0xff606060);  // Hover state

    // ========== LED Colors (for GR meters) ==========
    const juce::Colour LED_GREEN     (0xff00ff00);  // 0-3 dB
    const juce::Colour LED_YELLOW    (0xffffaa00);  // 3-8 dB
    const juce::Colour LED_RED       (0xffff3333);  // 8+ dB
    const juce::Colour LED_OFF       (0xff2a2a2a);  // LED off state

    // ========== Bypass States ==========
    const juce::Colour BYPASS_ON     (0xffdd3333);  // Red when bypassed (INACTIVE)
    const juce::Colour BYPASS_OFF    (0xff555555);  // Gray when active (ACTIVE - neutral)

    // ========== Helper Functions ==========

    /**
     * Get LED color based on dB reduction value.
     * @param reductionDB Gain reduction in dB (negative value)
     * @return Appropriate LED color (green/yellow/red)
     */
    inline juce::Colour getLEDColorForGR (float reductionDB)
    {
        float absGR = std::abs (reductionDB);

        if (absGR < 3.0f)
            return LED_GREEN;
        else if (absGR < 8.0f)
            return LED_YELLOW.interpolatedWith (LED_RED, (absGR - 3.0f) / 5.0f);
        else
            return LED_RED;
    }

    /**
     * Get interpolated color for gradient effects.
     * @param value Normalized value 0.0-1.0
     * @param colorLow Color at value 0.0
     * @param colorHigh Color at value 1.0
     * @return Interpolated color
     */
    inline juce::Colour getInterpolatedColor (float value,
                                               const juce::Colour& colorLow,
                                               const juce::Colour& colorHigh)
    {
        return colorLow.interpolatedWith (colorHigh, juce::jlimit (0.0f, 1.0f, value));
    }
}
