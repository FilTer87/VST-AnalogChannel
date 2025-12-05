/*
  ==============================================================================

    PresetBarComponent.cpp
    Created: 4 Dec 2024
    Author:  ParallelGrid

  ==============================================================================
*/

#include "PresetBarComponent.h"

//==============================================================================
PresetBarComponent::PresetBarComponent()
{
    // Preset ComboBox
    addAndMakeVisible(presetComboBox);
    presetComboBox.setTextWhenNothingSelected("<Default>");
    presetComboBox.onChange = [this]()
    {
        int selectedId = presetComboBox.getSelectedId();
        if (selectedId > 0)
        {
            juce::String presetName = presetComboBox.getItemText(selectedId - 1);
            loadPreset(presetName);
        }
    };

    // Save button
    addAndMakeVisible(savePresetButton);
    savePresetButton.setButtonText("Save");
    savePresetButton.onClick = [this]() { savePreset(); };

    // Delete button
    addAndMakeVisible(deletePresetButton);
    deletePresetButton.setButtonText("Delete");
    deletePresetButton.onClick = [this]() { deletePreset(); };

    // Master output label (hidden by default until enabled)
    masterOutputLabel.setText("Out", juce::dontSendNotification);
    masterOutputLabel.setJustificationType(juce::Justification::centredRight);

    // Master output slider (hidden by default until enabled)
    masterOutputSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    masterOutputSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    masterOutputSlider.setRange(-18.0, 18.0, 0.1);
    masterOutputSlider.setDoubleClickReturnValue(true, 0.0);
    masterOutputSlider.setTextValueSuffix(" dB");
}

PresetBarComponent::~PresetBarComponent()
{
    // Remove parameter listener if attached
    if (linkedAPVTS != nullptr && !masterOutputParamID.isEmpty())
    {
        linkedAPVTS->removeParameterListener(masterOutputParamID, this);
    }
}

//==============================================================================
// Core - Preset Management

void PresetBarComponent::setPluginName(const juce::String& name)
{
    pluginName = name;
    scanPresets();
}

void PresetBarComponent::scanPresets()
{
    presetComboBox.clear();

    auto presetsDir = getPresetsDirectory();
    auto presetFiles = presetsDir.findChildFiles(juce::File::findFiles, false, "*.vstpreset");

    int id = 1;
    for (const auto& file : presetFiles)
    {
        presetComboBox.addItem(file.getFileNameWithoutExtension(), id++);
    }

    // Update Delete button state
    deletePresetButton.setEnabled(presetComboBox.getNumItems() > 0);
}

juce::File PresetBarComponent::getPresetsDirectory()
{
    // Standard path: Documents/<PluginName>/Presets/
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    auto presetsDir = documentsDir.getChildFile(pluginName).getChildFile("Presets");

    // Create directory if it doesn't exist
    if (!presetsDir.exists())
        presetsDir.createDirectory();

    return presetsDir;
}

void PresetBarComponent::updatePresetDisplay()
{
    if (currentPresetName.isEmpty())
    {
        presetComboBox.setText("<Default>", juce::dontSendNotification);
    }
    else
    {
        juce::String displayName = hasUnsavedChanges ? "* " + currentPresetName : currentPresetName;
        presetComboBox.setText(displayName, juce::dontSendNotification);
    }
}

//==============================================================================
// Master Output

void PresetBarComponent::enableMasterOutput(juce::AudioProcessorValueTreeState& apvts,
                                           const juce::String& parameterID,
                                           float minDb,
                                           float maxDb)
{
    masterOutputEnabled = true;
    linkedAPVTS = &apvts;
    masterOutputParamID = parameterID;

    // Configure slider range
    masterOutputSlider.setRange(minDb, maxDb, 0.1);

    // Attach to APVTS
    masterOutputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, parameterID, masterOutputSlider);

    // Add parameter listener for unsaved changes tracking
    apvts.addParameterListener(parameterID, this);

    // Show components
    addAndMakeVisible(masterOutputLabel);
    addAndMakeVisible(masterOutputSlider);

    resized();
}

void PresetBarComponent::setMasterOutputLabel(const juce::String& label)
{
    masterOutputLabel.setText(label, juce::dontSendNotification);
}

//==============================================================================
// Custom Sections

void PresetBarComponent::setCustomLeftSection(juce::Component* component)
{
    customLeftSection = component;
    if (component != nullptr)
        addAndMakeVisible(component);
    resized();
}

void PresetBarComponent::setCustomCenterSection(juce::Component* component)
{
    customCenterSection = component;
    if (component != nullptr)
        addAndMakeVisible(component);
    resized();
}

void PresetBarComponent::setCustomRightSection(juce::Component* component)
{
    customRightSection = component;
    if (component != nullptr)
        addAndMakeVisible(component);
    resized();
}

//==============================================================================
// Layout

void PresetBarComponent::resized()
{
    auto area = getLocalBounds().reduced(6);

    const int buttonWidth = 60;
    const int buttonHeight = 28;
    const int presetComboWidth = 200;

    // Left side: Preset management
    presetComboBox.setBounds(area.removeFromLeft(presetComboWidth).withHeight(buttonHeight));
    area.removeFromLeft(6); // Spacing

    savePresetButton.setBounds(area.removeFromLeft(buttonWidth).withHeight(buttonHeight));
    area.removeFromLeft(4); // Spacing

    deletePresetButton.setBounds(area.removeFromLeft(buttonWidth).withHeight(buttonHeight));
    area.removeFromLeft(8); // Spacing

    // Custom left section (if present)
    if (customLeftSection != nullptr && customLeftSection->isVisible())
    {
        customLeftSection->setBounds(area.removeFromLeft(100).withHeight(buttonHeight));
        area.removeFromLeft(8); // Spacing
    }

    // Custom center section (if present)
    if (customCenterSection != nullptr && customCenterSection->isVisible())
    {
        customCenterSection->setBounds(area.removeFromLeft(100).withHeight(buttonHeight));
        area.removeFromLeft(8); // Spacing
    }

    // Right side: Master output (if enabled)
    if (masterOutputEnabled)
    {
        auto masterOutputWidth = 180;
        auto rightSection = area.removeFromRight(masterOutputWidth);

        masterOutputSlider.setBounds(rightSection.removeFromRight(150).withHeight(buttonHeight));
        rightSection.removeFromRight(4); // Spacing
        masterOutputLabel.setBounds(rightSection.withHeight(buttonHeight));
    }

    // Custom right section (if present)
    if (customRightSection != nullptr && customRightSection->isVisible())
    {
        auto rightCustom = area.removeFromRight(100);
        customRightSection->setBounds(rightCustom.withHeight(buttonHeight));
    }
}

void PresetBarComponent::paint(juce::Graphics& g)
{
    // Optional: draw background or border if needed
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

//==============================================================================
// APVTS Listener

void PresetBarComponent::parameterChanged(const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused(parameterID, newValue);

    // Mark as having unsaved changes when any parameter changes
    if (!currentPresetName.isEmpty() && !hasUnsavedChanges)
    {
        hasUnsavedChanges = true;
        updatePresetDisplay();
    }
}

//==============================================================================
// Preset Operations

void PresetBarComponent::savePreset()
{
    if (onGetState == nullptr)
    {
        jassertfalse; // Callback not set!
        return;
    }

    // If no preset selected, show "Save As" dialog
    if (currentPresetName.isEmpty())
    {
        // Show "Save As" dialog
        auto* saveDialog = new juce::AlertWindow("Save Preset", "Enter preset name:", juce::AlertWindow::NoIcon);
        saveDialog->addTextEditor("presetName", "", "Preset Name:");
        saveDialog->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        saveDialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        saveDialog->enterModalState(true, juce::ModalCallbackFunction::create([this, saveDialog](int result)
        {
            if (result == 1)
            {
                juce::String newPresetName = saveDialog->getTextEditorContents("presetName");
                if (newPresetName.isNotEmpty())
                {
                    // Save to file
                    auto presetsDir = getPresetsDirectory();
                    auto presetFile = presetsDir.getChildFile(newPresetName + ".vstpreset");

                    juce::MemoryBlock stateData;
                    onGetState(stateData);

                    if (presetFile.replaceWithData(stateData.getData(), stateData.getSize()))
                    {
                        currentPresetName = newPresetName;
                        hasUnsavedChanges = false;
                        scanPresets(); // Refresh list
                        updatePresetDisplay();
                    }
                }
            }
            delete saveDialog;
        }), true);
    }
    else
    {
        // Show confirmation dialog with "Yes" (overwrite) and "No, save as..."
        auto* confirmDialog = new juce::AlertWindow("Overwrite Preset",
                                                     "Do you want to overwrite the preset '" + currentPresetName + "'?",
                                                     juce::AlertWindow::QuestionIcon);
        confirmDialog->addButton("Yes", 1, juce::KeyPress(juce::KeyPress::returnKey));
        confirmDialog->addButton("No, save as...", 2);
        confirmDialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        confirmDialog->enterModalState(true, juce::ModalCallbackFunction::create([this, confirmDialog](int result)
        {
            if (result == 1)
            {
                // Overwrite existing preset
                auto presetsDir = getPresetsDirectory();
                auto presetFile = presetsDir.getChildFile(currentPresetName + ".vstpreset");

                juce::MemoryBlock stateData;
                onGetState(stateData);

                if (presetFile.replaceWithData(stateData.getData(), stateData.getSize()))
                {
                    hasUnsavedChanges = false;
                    updatePresetDisplay();
                }
                delete confirmDialog;
            }
            else if (result == 2)
            {
                // Save as new preset
                delete confirmDialog;

                auto* saveDialog = new juce::AlertWindow("Save Preset As", "Enter new preset name:", juce::AlertWindow::NoIcon);
                saveDialog->addTextEditor("presetName", currentPresetName, "Preset Name:");
                saveDialog->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
                saveDialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

                saveDialog->enterModalState(true, juce::ModalCallbackFunction::create([this, saveDialog](int result2)
                {
                    if (result2 == 1)
                    {
                        juce::String newPresetName = saveDialog->getTextEditorContents("presetName");
                        if (newPresetName.isNotEmpty())
                        {
                            auto presetsDir = getPresetsDirectory();
                            auto presetFile = presetsDir.getChildFile(newPresetName + ".vstpreset");

                            juce::MemoryBlock stateData;
                            onGetState(stateData);

                            if (presetFile.replaceWithData(stateData.getData(), stateData.getSize()))
                            {
                                currentPresetName = newPresetName;
                                hasUnsavedChanges = false;
                                scanPresets(); // Refresh list
                                updatePresetDisplay();
                            }
                        }
                    }
                    delete saveDialog;
                }), true);
            }
            else
            {
                delete confirmDialog;
            }
        }), true);
    }
}

void PresetBarComponent::loadPreset(const juce::String& presetName)
{
    if (onSetState == nullptr)
    {
        jassertfalse; // Callback not set!
        return;
    }

    auto presetsDir = getPresetsDirectory();
    auto presetFile = presetsDir.getChildFile(presetName + ".vstpreset");

    if (presetFile.existsAsFile())
    {
        juce::MemoryBlock stateData;
        if (presetFile.loadFileAsData(stateData))
        {
            onSetState(stateData.getData(), static_cast<int>(stateData.getSize()));
            currentPresetName = presetName;
            hasUnsavedChanges = false;
            updatePresetDisplay();
        }
    }
}

void PresetBarComponent::deletePreset()
{
    int selectedId = presetComboBox.getSelectedId();
    if (selectedId <= 0)
        return;

    juce::String presetName = presetComboBox.getItemText(selectedId - 1);

    // Show confirmation async
    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        "Delete Preset",
        "Are you sure you want to delete the preset '" + presetName + "'?",
        "Delete",
        "Cancel",
        nullptr,
        juce::ModalCallbackFunction::create([this, presetName](int result)
        {
            if (result == 1) // OK clicked
            {
                auto presetsDir = getPresetsDirectory();
                auto presetFile = presetsDir.getChildFile(presetName + ".vstpreset");

                if (presetFile.deleteFile())
                {
                    // If we deleted the currently loaded preset, reset
                    if (currentPresetName == presetName)
                    {
                        currentPresetName = juce::String();
                        hasUnsavedChanges = false;
                    }

                    scanPresets(); // Refresh list
                    updatePresetDisplay();
                }
            }
        })
    );
}
