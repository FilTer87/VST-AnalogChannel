/*
  ==============================================================================

    PeakMeter.h
    Vertical peak meter with dB scale
    Displays input/output levels with color gradient

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"

class PeakMeter : public juce::Component
{
public:
    PeakMeter()
    {
        // dB scale markings (0 dB at top is implicit)
        dbMarkers = { -3, -6, -12, -18, -24, -32, -48 };
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto meterBounds = bounds.reduced (2.0f);

        // Background
        g.setColour (AnalogChannelColors::BG_DARK);
        g.fillRoundedRectangle (meterBounds, 2.0f);

        // Draw dB scale markings inside the meter (overlaid on background)
        g.setColour (AnalogChannelColors::TEXT_MAIN.withAlpha (0.7f));
        g.setFont (juce::FontOptions (7.0f));

        for (int db : dbMarkers)
        {
            float y = dbToY (db, meterBounds.getHeight());

            // Draw text inside meter, centered
            auto textBounds = juce::Rectangle<float> (meterBounds.getX(), y - 5, meterBounds.getWidth(), 10);
            g.drawText (juce::String (db), textBounds.toNearestInt(), juce::Justification::centred);
        }

        // Draw meter bar
        float levelDB = juce::Decibels::gainToDecibels (peakLevel + 1e-10f);
        float barHeight = dbToHeight (levelDB, meterBounds.getHeight());

        if (barHeight > 0)
        {
            auto barBounds = meterBounds.withTop (meterBounds.getBottom() - barHeight);

            // Gradient fill (green → yellow → red)
            juce::ColourGradient gradient (AnalogChannelColors::LED_RED, 0, barBounds.getY(),
                                          AnalogChannelColors::LED_GREEN, 0, barBounds.getBottom(),
                                          false);
            gradient.addColour (0.5, AnalogChannelColors::LED_YELLOW);

            g.setGradientFill (gradient);
            g.fillRoundedRectangle (barBounds, 2.0f);
        }

        // Border
        g.setColour (AnalogChannelColors::BORDER_LIGHT);
        g.drawRoundedRectangle (meterBounds, 2.0f, 1.0f);
    }

    //==============================================================================
    /**
     * Set peak level (linear 0.0-1.0+).
     * @param level Peak level in linear scale
     */
    void setLevel (float level)
    {
        if (level != peakLevel)
        {
            peakLevel = level;
            repaint();
        }
    }

    float getLevel() const { return peakLevel; }

private:
    //==============================================================================
    /**
     * Convert dB value to Y position in meter.
     */
    float dbToY (float db, float height) const
    {
        // Map -60dB to 0dB → 0 to height
        float normalized = juce::jmap (db, -60.0f, 0.0f, 0.0f, 1.0f);
        return height * (1.0f - normalized);
    }

    /**
     * Convert dB value to bar height.
     */
    float dbToHeight (float db, float totalHeight) const
    {
        float clampedDB = juce::jlimit (-60.0f, 0.0f, db);
        float normalized = juce::jmap (clampedDB, -60.0f, 0.0f, 0.0f, 1.0f);
        return totalHeight * normalized;
    }

    //==============================================================================
    std::vector<int> dbMarkers;
    float peakLevel = 0.0f; // Linear 0.0-1.0+

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeakMeter)
};
