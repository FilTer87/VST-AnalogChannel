/*
  ==============================================================================

    LEDMeterStrip.h
    LED strip meter component for gain reduction display
    6-8 LEDs with gradient coloring (green → yellow → red)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"

class LEDMeterStrip : public juce::Component
{
public:
    LEDMeterStrip (int numLEDs = 8)
        : numberOfLEDs (numLEDs)
    {
        // Linear scale: 1-8 dB (one threshold per LED)
        ledThresholds.clear();
        for (int i = 0; i < numLEDs; ++i)
            ledThresholds.push_back (static_cast<float> (i + 1));
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float ledDiameter = 4.0f;  // Small circular LEDs
        const float spacing = (bounds.getWidth() - (numberOfLEDs * ledDiameter)) / (numberOfLEDs + 1);
        const float centerY = bounds.getHeight() / 2.0f;

        for (int i = 0; i < numberOfLEDs; ++i)
        {
            float x = spacing + i * (ledDiameter + spacing);
            float y = centerY - ledDiameter / 2.0f;

            // Determine if this LED should be lit based on current value
            bool isLit = (reductionDB >= ledThresholds[i]);

            // Determine LED color based on threshold
            juce::Colour ledColor;
            if (ledThresholds[i] < 3.0f)
                ledColor = AnalogChannelColors::LED_GREEN;
            else if (ledThresholds[i] < 8.0f)
                ledColor = AnalogChannelColors::LED_YELLOW;
            else
                ledColor = AnalogChannelColors::LED_RED;

            // Draw LED as circle
            if (isLit)
            {
                g.setColour (ledColor);
                g.fillEllipse (x, y, ledDiameter, ledDiameter);

                // Add subtle glow effect when lit
                g.setColour (ledColor.withAlpha (0.4f));
                g.drawEllipse (x - 0.5f, y - 0.5f, ledDiameter + 1.0f, ledDiameter + 1.0f, 1.0f);
            }
            else
            {
                // LED off state
                g.setColour (AnalogChannelColors::LED_OFF);
                g.fillEllipse (x, y, ledDiameter, ledDiameter);

                // Border
                g.setColour (AnalogChannelColors::BORDER_DARK);
                g.drawEllipse (x, y, ledDiameter, ledDiameter, 0.5f);
            }
        }

        // Draw dB scale labels below LEDs (show 1, 3, 5, 8)
        g.setColour (AnalogChannelColors::TEXT_DIM);
        g.setFont (juce::FontOptions (7.0f));

        // Show labels for: 1, 3, 5, 8 dB (indices 0, 2, 4, 7)
        std::vector<int> labelIndices = { 0, 2, 4, 7 };

        for (int idx : labelIndices)
        {
            if (idx < numberOfLEDs)
            {
                float x = spacing + idx * (ledDiameter + spacing);
                auto labelBounds = juce::Rectangle<float> (x - ledDiameter, bounds.getHeight() - 10.0f,
                                                            ledDiameter * 3, 10.0f);
                g.drawText (juce::String (static_cast<int> (ledThresholds[idx])),
                           labelBounds, juce::Justification::centred);
            }
        }
    }

    //==============================================================================
    /**
     * Set the current gain reduction value in dB (absolute value).
     * @param grDB Gain reduction in dB (positive value, e.g., 6.0 for -6dB)
     */
    void setValue (float grDB)
    {
        float newValue = std::abs (grDB);

        if (newValue != reductionDB)
        {
            reductionDB = newValue;
            repaint();
        }
    }

    /**
     * Get current value.
     */
    float getValue() const { return reductionDB; }

private:
    //==============================================================================
    int numberOfLEDs = 8;
    std::vector<float> ledThresholds;
    float reductionDB = 0.0f; // Absolute value in dB

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDMeterStrip)
};
