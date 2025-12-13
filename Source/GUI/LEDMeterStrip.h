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
    enum MeterType
    {
        Compressor,  // For Control-Comp and Style-Comp (1-8 dB, green: 1-2, yellow: 3-6, red: 7-8)
        OutStage     // For OutStage (0.1-10 dB, green: 0.1, yellow: 0.5-5, red: 7-10)
    };

    LEDMeterStrip (int numLEDs = 8, MeterType type = Compressor)
        : numberOfLEDs (numLEDs), meterType (type)
    {
        // Set thresholds based on meter type
        ledThresholds.clear();

        if (meterType == OutStage)
        {
            // OutStage scale: 0.1, 0.5, 1, 2, 3, 5, 7, 10 dB
            ledThresholds = { 0.1f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 7.0f, 10.0f };
        }
        else
        {
            // Compressor scale: 1-8 dB (linear)
            for (int i = 0; i < numLEDs; ++i)
                ledThresholds.push_back (static_cast<float> (i + 1));
        }
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

            // Determine LED color based on meter type and threshold
            juce::Colour ledColor;
            if (meterType == OutStage)
            {
                // OutStage colors: green: 0.1, yellow: 0.5-5, red: 7-10
                if (ledThresholds[i] <= 0.1f)
                    ledColor = AnalogChannelColors::LED_GREEN;
                else if (ledThresholds[i] <= 5.0f)
                    ledColor = AnalogChannelColors::LED_YELLOW;
                else
                    ledColor = AnalogChannelColors::LED_RED;
            }
            else
            {
                // Compressor colors: green: 1-2, yellow: 3-6, red: 7-8
                if (ledThresholds[i] <= 2.0f)
                    ledColor = AnalogChannelColors::LED_GREEN;
                else if (ledThresholds[i] <= 6.0f)
                    ledColor = AnalogChannelColors::LED_YELLOW;
                else
                    ledColor = AnalogChannelColors::LED_RED;
            }

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

        // Draw dB scale labels below LEDs
        g.setColour (AnalogChannelColors::TEXT_DIM);
        g.setFont (juce::FontOptions (7.0f));

        // Select which LEDs to label based on meter type
        std::vector<int> labelIndices;
        if (meterType == OutStage)
        {
            // OutStage: show 0.1, 1, 3, 7 (indices 0, 2, 4, 6)
            labelIndices = { 0, 2, 4, 6 };
        }
        else
        {
            // Compressor: show 1, 3, 5, 8 (indices 0, 2, 4, 7)
            labelIndices = { 0, 2, 4, 7 };
        }

        for (int idx : labelIndices)
        {
            if (idx < numberOfLEDs)
            {
                float x = spacing + idx * (ledDiameter + spacing);
                auto labelBounds = juce::Rectangle<float> (x - ledDiameter, bounds.getHeight() - 10.0f,
                                                            ledDiameter * 3, 10.0f);

                // Format label based on value (show decimal for < 1.0)
                juce::String label;
                if (ledThresholds[idx] < 1.0f)
                    label = juce::String (ledThresholds[idx], 1);  // Show 1 decimal: "0.1", "0.5"
                else
                    label = juce::String (static_cast<int> (ledThresholds[idx]));  // Show integer: "1", "3", "7"

                g.drawText (label, labelBounds, juce::Justification::centred);
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
    MeterType meterType;
    std::vector<float> ledThresholds;
    float reductionDB = 0.0f; // Absolute value in dB

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDMeterStrip)
};
