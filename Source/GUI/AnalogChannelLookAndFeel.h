/*
  ==============================================================================

    AnalogChannelLookAndFeel.h
    Custom LookAndFeel for AnalogChannel GUI
    Hardware-inspired knobs with line indicator (no excessive shadows)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Colors.h"

class AnalogChannelLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AnalogChannelLookAndFeel()
    {
        // Set default colors from our palette
        setColour (juce::Slider::backgroundColourId, AnalogChannelColors::KNOB_BODY);
        setColour (juce::Slider::thumbColourId, AnalogChannelColors::KNOB_INDICATOR);
        setColour (juce::Slider::trackColourId, AnalogChannelColors::KNOB_HIGHLIGHT);
        setColour (juce::Slider::textBoxTextColourId, AnalogChannelColors::TEXT_MAIN);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId, AnalogChannelColors::BORDER_LIGHT.withAlpha (0.3f));

        // Force Label colors globally for slider textboxes
        setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }

    //==============================================================================
    // Custom Slider TextBox Label (only for knob value display)
    //==============================================================================
    juce::Label* createSliderTextBox (juce::Slider& slider) override
    {
        auto* label = LookAndFeel_V4::createSliderTextBox (slider);
        label->setFont (juce::Font (9.0f));  // Smaller font for values (9pt)
        label->setJustificationType (juce::Justification::centred);
        label->setColour (juce::Label::textColourId, AnalogChannelColors::TEXT_MAIN);
        label->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        label->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
        label->setColour (juce::Label::backgroundWhenEditingColourId, AnalogChannelColors::BG_DARK);
        label->setColour (juce::Label::outlineWhenEditingColourId, AnalogChannelColors::BORDER_LIGHT);
        label->setColour (juce::TextEditor::textColourId, AnalogChannelColors::TEXT_MAIN);
        label->setColour (juce::TextEditor::backgroundColourId, AnalogChannelColors::BG_DARK);
        label->setColour (juce::TextEditor::outlineColourId, AnalogChannelColors::BORDER_LIGHT);
        label->setColour (juce::TextEditor::highlightColourId, AnalogChannelColors::BUTTON_ON.withAlpha (0.3f));
        return label;
    }

    //==============================================================================
    // Override getSliderLayout to force smaller textbox and no outline
    //==============================================================================
    juce::Slider::SliderLayout getSliderLayout (juce::Slider& slider) override
    {
        auto layout = LookAndFeel_V4::getSliderLayout (slider);

        // Keep the same layout but we'll handle the rendering differently
        return layout;
    }

    //==============================================================================
    // Override drawLabel only for slider value labels (attached to parent Slider)
    //==============================================================================
    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        // Check if this label is attached to a Slider (value label)
        bool isSliderLabel = dynamic_cast<juce::Slider*>(label.getParentComponent()) != nullptr;

        if (isSliderLabel)
        {
            // Custom rendering for slider value labels: small font, no border
            g.fillAll (juce::Colours::transparentBlack);

            if (!label.isBeingEdited())
            {
                const float alpha = label.isEnabled() ? 1.0f : 0.5f;
                g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
                g.setFont (juce::Font (9.0f));  // 9pt for slider values

                auto textArea = label.getLocalBounds();
                g.drawFittedText (label.getText(), textArea, label.getJustificationType(), 1);
            }
        }
        else
        {
            // Default rendering for all other labels (section names, knob labels, etc.)
            LookAndFeel_V4::drawLabel (g, label);
        }
    }

    //==============================================================================
    // Custom Rotary Slider (Knob) Drawing
    //==============================================================================
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10.0f);

        // Force circular aspect ratio (1:1)
        auto size = juce::jmin (bounds.getWidth(), bounds.getHeight());
        auto knobBounds = juce::Rectangle<float> (size, size).withCentre (bounds.getCentre());

        auto radius = size / 2.0f;
        auto centre = knobBounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Draw knob body (metallic circle with subtle gradient)
        {
            juce::ColourGradient gradient (AnalogChannelColors::KNOB_HIGHLIGHT, centre.x, centre.y - radius,
                                           AnalogChannelColors::KNOB_SHADOW, centre.x, centre.y + radius,
                                           false);
            g.setGradientFill (gradient);
            g.fillEllipse (knobBounds);

            // Border
            g.setColour (AnalogChannelColors::BORDER_DARK);
            g.drawEllipse (knobBounds, 1.5f);
        }

        // Draw indicator line (orange, from center to edge)
        {
            juce::Path indicator;
            auto indicatorLength = radius * 0.7f;
            auto indicatorThickness = 2.5f;

            indicator.addRectangle (-indicatorThickness * 0.5f, -radius + 5.0f,
                                    indicatorThickness, indicatorLength);

            g.setColour (AnalogChannelColors::KNOB_INDICATOR);
            g.fillPath (indicator, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
        }

        // Draw center dot (subtle detail)
        {
            auto dotRadius = 3.0f;
            g.setColour (AnalogChannelColors::BORDER_DARK);
            g.fillEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
        }
    }

    //==============================================================================
    // Custom Toggle Button (for bypass switches)
    //==============================================================================
    void drawToggleButton (juce::Graphics& g,
                           juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);

        // Determine if this is a bypass button or filter/param button
        auto buttonText = button.getButtonText();
        bool isBypassButton = buttonText.contains ("BYPASS") ||
                               buttonText.contains ("ACTIVE") ||
                               buttonText.contains ("INACTIVE") ||
                               buttonText.contains ("ENABLED") ||
                               buttonText.contains ("DISABLED");

        // Background - use orange for filter buttons, green for bypass
        auto onColor = isBypassButton ? AnalogChannelColors::BYPASS_ON : AnalogChannelColors::BUTTON_ON;
        auto offColor = isBypassButton ? AnalogChannelColors::BYPASS_OFF : AnalogChannelColors::BUTTON_OFF;

        g.setColour (button.getToggleState() ? onColor : offColor);
        g.fillRoundedRectangle (bounds, 3.0f);

        // Border
        g.setColour (AnalogChannelColors::BORDER_LIGHT);
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

        // Text - when button is ON (orange), use dark text for better readability
        // But keep white text for bypass buttons (ACTIVE/INACTIVE)
        auto textColor = AnalogChannelColors::TEXT_MAIN;
        if (button.getToggleState() && !isBypassButton)
            textColor = juce::Colour (0xff1a1a1a);  // Dark text for active orange buttons

        g.setColour (textColor);
        g.setFont (10.0f);
        g.drawText (button.getButtonText(), bounds, juce::Justification::centred);
    }

    //==============================================================================
    // Custom ComboBox (for algorithm selectors)
    //==============================================================================
    void drawComboBox (juce::Graphics& g,
                       int width, int height,
                       bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat();

        // Background
        g.setColour (AnalogChannelColors::PANEL_BG);
        g.fillRoundedRectangle (bounds, 3.0f);

        // Border
        g.setColour (AnalogChannelColors::BORDER_LIGHT);
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

        // Arrow indicator
        juce::Path arrow;
        arrow.addTriangle (buttonX + buttonW * 0.3f, buttonY + buttonH * 0.4f,
                          buttonX + buttonW * 0.7f, buttonY + buttonH * 0.4f,
                          buttonX + buttonW * 0.5f, buttonY + buttonH * 0.7f);

        g.setColour (AnalogChannelColors::TEXT_DIM);
        g.fillPath (arrow);
    }

    //==============================================================================
    // Custom TextButton (for toggle button groups like Fast/Normal)
    //==============================================================================
    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);

        // Background color based on toggle state
        auto bgColor = button.getToggleState() ? AnalogChannelColors::BUTTON_ON : AnalogChannelColors::BUTTON_OFF;

        if (shouldDrawButtonAsHighlighted && !button.getToggleState())
            bgColor = AnalogChannelColors::BUTTON_HOVER;

        g.setColour (bgColor);
        g.fillRoundedRectangle (bounds, 3.0f);

        // Border
        g.setColour (AnalogChannelColors::BORDER_LIGHT);
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
    }

    void drawButtonText (juce::Graphics& g,
                        juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override
    {
        // When button is ON (orange), use dark text for better readability
        auto textColor = button.getToggleState() ? juce::Colour (0xff1a1a1a) : AnalogChannelColors::TEXT_MAIN;

        g.setColour (textColor);
        g.setFont (11.0f);
        g.drawText (button.getButtonText(),
                   button.getLocalBounds(),
                   juce::Justification::centred);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogChannelLookAndFeel)
};

//==============================================================================
// Pointer-style LookAndFeel for frequency selector knobs
//==============================================================================
class PointerKnobLookAndFeel : public AnalogChannelLookAndFeel
{
public:
    PointerKnobLookAndFeel() = default;

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10.0f);

        // Force circular aspect ratio (1:1)
        auto size = juce::jmin (bounds.getWidth(), bounds.getHeight());
        auto knobBounds = juce::Rectangle<float> (size, size).withCentre (bounds.getCentre());

        auto radius = size / 2.0f;
        auto centre = knobBounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Draw knob body (metallic circle with subtle gradient)
        {
            juce::ColourGradient gradient (AnalogChannelColors::KNOB_HIGHLIGHT, centre.x, centre.y - radius,
                                           AnalogChannelColors::KNOB_SHADOW, centre.x, centre.y + radius,
                                           false);
            g.setGradientFill (gradient);
            g.fillEllipse (knobBounds);

            // Border
            g.setColour (AnalogChannelColors::BORDER_DARK);
            g.drawEllipse (knobBounds, 1.5f);
        }

        // Draw pointer indicator (triangle pointing outward)
        {
            juce::Path pointer;
            auto pointerLength = radius * 0.6f;
            auto pointerWidth = 6.0f;

            // Triangle pointing up (from center outward)
            pointer.addTriangle (0.0f, -radius + 3.0f,                    // Tip at edge
                                -pointerWidth * 0.5f, -radius + 3.0f + pointerLength,  // Bottom left
                                 pointerWidth * 0.5f, -radius + 3.0f + pointerLength); // Bottom right

            g.setColour (AnalogChannelColors::KNOB_INDICATOR);
            g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
        }

        // Draw center dot (subtle detail)
        {
            auto dotRadius = 3.0f;
            g.setColour (AnalogChannelColors::BORDER_DARK);
            g.fillEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
        }
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PointerKnobLookAndFeel)
};

//==============================================================================
// Color-coded LookAndFeel classes for EQ knobs with custom center colors
//==============================================================================
class ColoredKnobLookAndFeel : public AnalogChannelLookAndFeel
{
public:
    ColoredKnobLookAndFeel (juce::Colour centerColor) : centerDotColor (centerColor) {}

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10.0f);

        // Force circular aspect ratio (1:1)
        auto size = juce::jmin (bounds.getWidth(), bounds.getHeight());
        auto knobBounds = juce::Rectangle<float> (size, size).withCentre (bounds.getCentre());

        auto radius = size / 2.0f;
        auto centre = knobBounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Draw knob body (metallic circle with subtle gradient)
        {
            juce::ColourGradient gradient (AnalogChannelColors::KNOB_HIGHLIGHT, centre.x, centre.y - radius,
                                           AnalogChannelColors::KNOB_SHADOW, centre.x, centre.y + radius,
                                           false);
            g.setGradientFill (gradient);
            g.fillEllipse (knobBounds);

            // Border
            g.setColour (AnalogChannelColors::BORDER_DARK);
            g.drawEllipse (knobBounds, 1.5f);
        }

        // Draw indicator line (orange, from center to edge)
        {
            juce::Path indicator;
            auto indicatorLength = radius * 0.7f;
            auto indicatorThickness = 2.5f;

            indicator.addRectangle (-indicatorThickness * 0.5f, -radius + 5.0f,
                                    indicatorThickness, indicatorLength);

            g.setColour (AnalogChannelColors::KNOB_INDICATOR);
            g.fillPath (indicator, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
        }

        // Draw colored center dot
        {
            auto dotRadius = 3.0f;  // Same size as standard knobs
            g.setColour (centerDotColor);
            g.fillEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);

            // Subtle border around the colored dot
            g.setColour (AnalogChannelColors::BORDER_DARK.withAlpha (0.5f));
            g.drawEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f, 0.5f);
        }
    }

private:
    juce::Colour centerDotColor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColoredKnobLookAndFeel)
};

// Pointer-style with custom center color for frequency knobs
class ColoredPointerKnobLookAndFeel : public PointerKnobLookAndFeel
{
public:
    ColoredPointerKnobLookAndFeel (juce::Colour centerColor) : centerDotColor (centerColor) {}

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10.0f);

        // Force circular aspect ratio (1:1)
        auto size = juce::jmin (bounds.getWidth(), bounds.getHeight());
        auto knobBounds = juce::Rectangle<float> (size, size).withCentre (bounds.getCentre());

        auto radius = size / 2.0f;
        auto centre = knobBounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Draw knob body (metallic circle with subtle gradient)
        {
            juce::ColourGradient gradient (AnalogChannelColors::KNOB_HIGHLIGHT, centre.x, centre.y - radius,
                                           AnalogChannelColors::KNOB_SHADOW, centre.x, centre.y + radius,
                                           false);
            g.setGradientFill (gradient);
            g.fillEllipse (knobBounds);

            // Border
            g.setColour (AnalogChannelColors::BORDER_DARK);
            g.drawEllipse (knobBounds, 1.5f);
        }

        // Draw pointer indicator (triangle pointing outward)
        {
            juce::Path pointer;
            auto pointerLength = radius * 0.6f;
            auto pointerWidth = 6.0f;

            // Triangle pointing up (from center outward)
            pointer.addTriangle (0.0f, -radius + 3.0f,                    // Tip at edge
                                -pointerWidth * 0.5f, -radius + 3.0f + pointerLength,  // Bottom left
                                 pointerWidth * 0.5f, -radius + 3.0f + pointerLength); // Bottom right

            g.setColour (AnalogChannelColors::KNOB_INDICATOR);
            g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
        }

        // Draw colored center dot (smaller for small knobs)
        {
            auto dotRadius = 2.0f;  // Reduced size for small knobs (Mid Cut, Makeup, Mix)
            g.setColour (centerDotColor);
            g.fillEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);

            // Subtle border around the colored dot
            g.setColour (AnalogChannelColors::BORDER_DARK.withAlpha (0.5f));
            g.drawEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f, 0.5f);
        }
    }

private:
    juce::Colour centerDotColor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColoredPointerKnobLookAndFeel)
};

//==============================================================================
// Dynamic colored knob that can change color at runtime
//==============================================================================
class DynamicColoredKnobLookAndFeel : public AnalogChannelLookAndFeel
{
public:
    DynamicColoredKnobLookAndFeel (juce::Colour initialColor = juce::Colour (0xff1a1a1a))
        : centerDotColor (initialColor) {}

    void setCenterColor (juce::Colour newColor)
    {
        centerDotColor = newColor;
    }

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10.0f);

        // Force circular aspect ratio (1:1)
        auto size = juce::jmin (bounds.getWidth(), bounds.getHeight());
        auto knobBounds = juce::Rectangle<float> (size, size).withCentre (bounds.getCentre());

        auto radius = size / 2.0f;
        auto centre = knobBounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Draw knob body (metallic circle with subtle gradient)
        {
            juce::ColourGradient gradient (AnalogChannelColors::KNOB_HIGHLIGHT, centre.x, centre.y - radius,
                                           AnalogChannelColors::KNOB_SHADOW, centre.x, centre.y + radius,
                                           false);
            g.setGradientFill (gradient);
            g.fillEllipse (knobBounds);

            // Border
            g.setColour (AnalogChannelColors::BORDER_DARK);
            g.drawEllipse (knobBounds, 1.5f);
        }

        // Draw indicator line (orange, from center to edge)
        {
            juce::Path indicator;
            auto indicatorLength = radius * 0.7f;
            auto indicatorThickness = 2.5f;

            indicator.addRectangle (-indicatorThickness * 0.5f, -radius + 5.0f,
                                    indicatorThickness, indicatorLength);

            g.setColour (AnalogChannelColors::KNOB_INDICATOR);
            g.fillPath (indicator, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
        }

        // Draw colored center dot
        {
            auto dotRadius = 3.0f;  // Same size as standard knobs
            g.setColour (centerDotColor);
            g.fillEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);

            // Subtle border around the colored dot
            g.setColour (AnalogChannelColors::BORDER_DARK.withAlpha (0.5f));
            g.drawEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f, 0.5f);
        }
    }

private:
    juce::Colour centerDotColor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicColoredKnobLookAndFeel)
};
