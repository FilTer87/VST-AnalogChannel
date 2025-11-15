/*
  ==============================================================================

    PluginProcessor.h
    AnalogChannel VST3 Channel Strip Plugin

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

    ---

    This plugin incorporates algorithms from third-party sources.
    See CREDITS.md for complete attribution and licensing information.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Sections/PreInputSection.h"
#include "Sections/FilterSection.h"
#include "Sections/ControlCompSection.h"
#include "Sections/LowDynamicSection.h"
#include "Sections/EQSection.h"
#include "Sections/StyleCompSection.h"
#include "Sections/ConsoleSection.h"
#include "Sections/OutStageSection.h"
#include "Sections/VolumeSection.h"
#include "ChannelVariation.h"

//==============================================================================
/**
*/
class AnalogChannelAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AnalogChannelAudioProcessor();
    ~AnalogChannelAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter Access
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

    //==============================================================================
    // Metering - GUI Access (thread-safe via atomics)

    // Peak meters (linear 0.0-1.0+, GUI converts to dB)
    float getInputPeakLeft() const { return inputPeakLeft.load (std::memory_order_relaxed); }
    float getInputPeakRight() const { return inputPeakRight.load (std::memory_order_relaxed); }
    float getOutputPeakLeft() const { return outputPeakLeft.load (std::memory_order_relaxed); }
    float getOutputPeakRight() const { return outputPeakRight.load (std::memory_order_relaxed); }

    // GR meters (dB, negative values = reduction)
    float getControlCompGRLeft() const { return controlCompGRLeft.load (std::memory_order_relaxed); }
    float getControlCompGRRight() const { return controlCompGRRight.load (std::memory_order_relaxed); }
    float getStyleCompGRLeft() const { return styleCompGRLeft.load (std::memory_order_relaxed); }
    float getStyleCompGRRight() const { return styleCompGRRight.load (std::memory_order_relaxed); }
    float getOutStageGRLeft() const { return outStageGRLeft.load (std::memory_order_relaxed); }
    float getOutStageGRRight() const { return outStageGRRight.load (std::memory_order_relaxed); }

private:
    //==============================================================================
    // Parameter Management
    juce::AudioProcessorValueTreeState parameters;

    // Helper function to create all parameters
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Update all sections with current parameter values
    void updateAllSections();

    //==============================================================================
    // Processing Sections - Dual Mono (index 0 = left, 1 = right)
    PreInputSection preInput[2];
    FilterSection filters[2];
    ControlCompSection controlComp[2];
    LowDynamicSection lowDynamic[2];
    EQSection eq[2];
    StyleCompSection styleComp[2];
    ConsoleSection console[2];
    OutStageSection outStage[2];
    VolumeSection volume[2];

    //==============================================================================
    // Metering System

    // Atomic variables for GUI access (thread-safe, lock-free)
    std::atomic<float> inputPeakLeft{0.0f};
    std::atomic<float> inputPeakRight{0.0f};
    std::atomic<float> outputPeakLeft{0.0f};
    std::atomic<float> outputPeakRight{0.0f};
    std::atomic<float> controlCompGRLeft{0.0f};
    std::atomic<float> controlCompGRRight{0.0f};
    std::atomic<float> styleCompGRLeft{0.0f};
    std::atomic<float> styleCompGRRight{0.0f};
    std::atomic<float> outStageGRLeft{0.0f};
    std::atomic<float> outStageGRRight{0.0f};

    // Processing thread state (not thread-safe, used only in processBlock)
    float inputPeakStateLeft = 0.0f;
    float inputPeakStateRight = 0.0f;
    float outputPeakStateLeft = 0.0f;
    float outputPeakStateRight = 0.0f;
    float outStageInputRMSLeft = 0.0f;
    float outStageInputRMSRight = 0.0f;
    float outStageOutputRMSLeft = 0.0f;
    float outStageOutputRMSRight = 0.0f;
    float outStageGRSmoothLeft = 0.0f;
    float outStageGRSmoothRight = 0.0f;

    // Metering coefficients (calculated in prepareToPlay)
    float peakDecayCoeff = 0.0f;
    float outStageAttackCoeff = 0.0f;
    float outStageReleaseCoeff = 0.0f;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogChannelAudioProcessor)
};
