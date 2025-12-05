/*
  ==============================================================================

    PresetBarComponent.h
    Created: 4 Dec 2024
    Author:  ParallelGrid

    Reusable preset management bar component for JUCE plugins.
    Handles preset save/load/delete with .vstpreset format.
    Optional master output slider and custom component sections.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * Reusable preset management bar component.
 *
 * Features:
 * - Preset save/load/delete with .vstpreset format
 * - Automatic unsaved changes tracking (*)
 * - Optional master output slider (linked to APVTS parameter)
 * - Custom component sections (left/center/right) for plugin-specific controls
 *
 * Usage:
 *   PresetBarComponent presetBar;
 *   presetBar.setPluginName("MyPlugin");
 *   presetBar.onGetState = [](juce::MemoryBlock& data) { processor.getStateInformation(data); };
 *   presetBar.onSetState = [](const void* data, int size) { processor.setStateInformation(data, size); };
 *   presetBar.enableMasterOutput(apvts, "masterOutput");
 */
class PresetBarComponent : public juce::Component,
                          private juce::AudioProcessorValueTreeState::Listener
{
public:
    PresetBarComponent();
    ~PresetBarComponent() override;

    //==============================================================================
    // CORE - Preset Management (always required)

    /** Set the plugin name for preset folder (Documents/<PluginName>/Presets/) */
    void setPluginName(const juce::String& name);

    /** Callback to get plugin state for saving preset */
    std::function<void(juce::MemoryBlock&)> onGetState;

    /** Callback to set plugin state when loading preset */
    std::function<void(const void*, int)> onSetState;

    /** Refresh preset list (call after external preset changes) */
    void scanPresets();

    //==============================================================================
    // MASTER OUTPUT (optional)

    /**
     * Enable tracking of all APVTS parameters for unsaved changes detection.
     * Call this to monitor all parameter changes and show asterisk (*) when modified.
     */
    void enableParameterTracking(juce::AudioProcessorValueTreeState& apvts);

    /** Enable master output slider linked to APVTS parameter */
    void enableMasterOutput(juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& parameterID,
                           float minDb = -18.0f,
                           float maxDb = 18.0f);

    /** Set master output label text (default: "Out") */
    void setMasterOutputLabel(const juce::String& label);

    //==============================================================================
    // CUSTOM SECTIONS (optional)
    // Add plugin-specific components in reserved areas

    /** Add custom component to left section (between delete button and master output) */
    void setCustomLeftSection(juce::Component* component);

    /** Add custom component to center section (between presets and master output) */
    void setCustomCenterSection(juce::Component* component);

    /** Add custom component to right section (after master output) */
    void setCustomRightSection(juce::Component* component);

    //==============================================================================
    // Component overrides
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    //==============================================================================
    // Core preset components
    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton;
    juce::TextButton deletePresetButton;

    // Master output (optional)
    bool masterOutputEnabled = false;
    juce::Label masterOutputLabel;
    juce::Slider masterOutputSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterOutputAttachment;
    juce::AudioProcessorValueTreeState* linkedAPVTS = nullptr;
    juce::String masterOutputParamID;

    // Parameter tracking
    bool parameterTrackingEnabled = false;
    juce::StringArray trackedParameterIDs;

    // Custom sections (optional)
    juce::Component* customLeftSection = nullptr;
    juce::Component* customCenterSection = nullptr;
    juce::Component* customRightSection = nullptr;

    //==============================================================================
    // Preset management state
    juce::String pluginName = "Plugin";
    juce::String currentPresetName;
    bool hasUnsavedChanges = false;

    // Preset operations
    void savePreset();
    void loadPreset(const juce::String& presetName);
    void deletePreset();
    void updatePresetDisplay();
    juce::File getPresetsDirectory();

    // APVTS Listener - track parameter changes for unsaved changes indicator
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBarComponent)
};
