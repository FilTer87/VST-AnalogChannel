/*
  ==============================================================================

    PluginProcessor.cpp
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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AnalogChannelAudioProcessor::AnalogChannelAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
     :
#endif
       parameters (*this, nullptr, juce::Identifier ("AnalogChannel"), createParameterLayout())
{
}

AnalogChannelAudioProcessor::~AnalogChannelAudioProcessor()
{
}

//==============================================================================
const juce::String AnalogChannelAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AnalogChannelAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AnalogChannelAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AnalogChannelAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AnalogChannelAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AnalogChannelAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AnalogChannelAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AnalogChannelAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AnalogChannelAudioProcessor::getProgramName (int index)
{
    return {};
}

void AnalogChannelAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AnalogChannelAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    // Initialize all sections with sample rate (dual-mono: left and right)
    for (int ch = 0; ch < 2; ++ch)
    {
        preInput[ch].setSampleRate (sampleRate);
        filters[ch].setSampleRate (sampleRate);
        controlComp[ch].setSampleRate (sampleRate);
        eq[ch].setSampleRate (sampleRate);
        styleComp[ch].setSampleRate (sampleRate);
        console[ch].setSampleRate (sampleRate);
        outStage[ch].setSampleRate (sampleRate);
        volume[ch].setSampleRate (sampleRate);
    }

    // Update all sections with current parameter values
    updateAllSections();

    // Initialize metering ballistics
    peakDecayCoeff = std::exp (-1.0f / (0.2f * static_cast<float> (sampleRate)));  // 200ms decay
    outStageAttackCoeff = std::exp (-1.0f / (0.01f * static_cast<float> (sampleRate)));  // 10ms attack
    outStageReleaseCoeff = std::exp (-1.0f / (0.05f * static_cast<float> (sampleRate)));  // 50ms release

    // Reset meter state
    inputPeakStateLeft = inputPeakStateRight = 0.0f;
    outputPeakStateLeft = outputPeakStateRight = 0.0f;
    outStageInputRMSLeft = outStageInputRMSRight = 0.0f;
    outStageOutputRMSLeft = outStageOutputRMSRight = 0.0f;
    outStageGRSmoothLeft = outStageGRSmoothRight = 0.0f;
}

void AnalogChannelAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AnalogChannelAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AnalogChannelAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused (midiMessages);

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any extra output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update all sections with current parameter values
    updateAllSections();

    // Process stereo channels independently (dual-mono)
    // We support up to 2 channels (stereo)
    const int numChannelsToProcess = juce::jmin (2, totalNumInputChannels);

    // Reset RMS accumulators at start of buffer
    outStageInputRMSLeft = outStageInputRMSRight = 0.0f;
    outStageOutputRMSLeft = outStageOutputRMSRight = 0.0f;

    for (int channel = 0; channel < numChannelsToProcess; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        const int numSamples = buffer.getNumSamples();

        // Process each sample through the signal chain
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float signal = channelData[sample];

            // === INPUT PEAK METERING ===
            {
                float inputLevel = std::abs (signal);
                float& peakState = (channel == 0) ? inputPeakStateLeft : inputPeakStateRight;

                if (inputLevel > peakState)
                    peakState = inputLevel;  // Instant attack
                else
                    peakState *= peakDecayCoeff;  // Exponential decay

                if (channel == 0)
                    inputPeakLeft.store (peakState, std::memory_order_relaxed);
                else
                    inputPeakRight.store (peakState, std::memory_order_relaxed);
            }

            // Signal flow: 8 sections in series
            signal = preInput[channel].process (signal);
            signal = filters[channel].process (signal);
            signal = controlComp[channel].process (signal);
            signal = eq[channel].process (signal);
            signal = styleComp[channel].process (signal);
            signal = console[channel].process (signal);

            // === OUTSTAGE GR DETECTION (accumulate RMS) ===
            float outStageInput = signal;
            signal = outStage[channel].process (signal);

            // Accumulate squared values for RMS calculation
            if (channel == 0)
            {
                outStageInputRMSLeft += outStageInput * outStageInput;
                outStageOutputRMSLeft += signal * signal;
            }
            else
            {
                outStageInputRMSRight += outStageInput * outStageInput;
                outStageOutputRMSRight += signal * signal;
            }

            signal = volume[channel].process (signal);

            // === OUTPUT PEAK METERING ===
            {
                float outputLevel = std::abs (signal);
                float& outPeakState = (channel == 0) ? outputPeakStateLeft : outputPeakStateRight;

                if (outputLevel > outPeakState)
                    outPeakState = outputLevel;  // Instant attack
                else
                    outPeakState *= peakDecayCoeff;  // Exponential decay

                if (channel == 0)
                    outputPeakLeft.store (outPeakState, std::memory_order_relaxed);
                else
                    outputPeakRight.store (outPeakState, std::memory_order_relaxed);
            }

            channelData[sample] = signal;
        }

        // === COMPRESSOR GR METERS (once per buffer, per channel) ===
        if (channel == 0)
        {
            controlCompGRLeft.store (controlComp[0].getGainReductionDB(), std::memory_order_relaxed);
            styleCompGRLeft.store (styleComp[0].getGainReductionDB(), std::memory_order_relaxed);
        }
        else
        {
            controlCompGRRight.store (controlComp[1].getGainReductionDB(), std::memory_order_relaxed);
            styleCompGRRight.store (styleComp[1].getGainReductionDB(), std::memory_order_relaxed);
        }
    }

    // === OUTSTAGE GR DETECTION (once per buffer, after all channels) ===
    const int numSamples = buffer.getNumSamples();

    // Left channel
    float inputRMS_L = std::sqrt (outStageInputRMSLeft / numSamples);
    float outputRMS_L = std::sqrt (outStageOutputRMSLeft / numSamples);
    float inputDB_L = juce::Decibels::gainToDecibels (inputRMS_L + 1e-10f);
    float outputDB_L = juce::Decibels::gainToDecibels (outputRMS_L + 1e-10f);
    float grDB_L = outputDB_L - inputDB_L;  // Negative if reducing
    float target_L = (grDB_L < -0.2f) ? 1.0f : 0.0f;  // 1.0 = active, 0.0 = inactive

    // Smooth with attack/release
    if (target_L > outStageGRSmoothLeft)
        outStageGRSmoothLeft += (target_L - outStageGRSmoothLeft) * (1.0f - outStageAttackCoeff);
    else
        outStageGRSmoothLeft += (target_L - outStageGRSmoothLeft) * (1.0f - outStageReleaseCoeff);

    outStageGRActiveLeft.store (outStageGRSmoothLeft > 0.5f, std::memory_order_relaxed);

    // Right channel (if stereo)
    if (numChannelsToProcess > 1)
    {
        float inputRMS_R = std::sqrt (outStageInputRMSRight / numSamples);
        float outputRMS_R = std::sqrt (outStageOutputRMSRight / numSamples);
        float inputDB_R = juce::Decibels::gainToDecibels (inputRMS_R + 1e-10f);
        float outputDB_R = juce::Decibels::gainToDecibels (outputRMS_R + 1e-10f);
        float grDB_R = outputDB_R - inputDB_R;
        float target_R = (grDB_R < -0.2f) ? 1.0f : 0.0f;

        if (target_R > outStageGRSmoothRight)
            outStageGRSmoothRight += (target_R - outStageGRSmoothRight) * (1.0f - outStageAttackCoeff);
        else
            outStageGRSmoothRight += (target_R - outStageGRSmoothRight) * (1.0f - outStageReleaseCoeff);

        outStageGRActiveRight.store (outStageGRSmoothRight > 0.5f, std::memory_order_relaxed);
    }
}

//==============================================================================
bool AnalogChannelAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AnalogChannelAudioProcessor::createEditor()
{
    return new AnalogChannelAudioProcessorEditor (*this);
}

//==============================================================================
void AnalogChannelAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save all parameters from AudioProcessorValueTreeState to memory block
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AnalogChannelAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore all parameters from memory block to AudioProcessorValueTreeState
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
void AnalogChannelAudioProcessor::updateAllSections()
{
    // Get parameter values
    auto preInputAlgo = parameters.getRawParameterValue ("preInputAlgo");
    auto preInputDrive = parameters.getRawParameterValue ("preInputDrive");
    auto preInputBypass = parameters.getRawParameterValue ("preInputBypass");

    auto hpfFreq = parameters.getRawParameterValue ("hpfFreq");
    auto hpfSlope = parameters.getRawParameterValue ("hpfSlope");
    auto hpfQ = parameters.getRawParameterValue ("hpfQ");
    auto lpfFreq = parameters.getRawParameterValue ("lpfFreq");
    auto lpfSlope = parameters.getRawParameterValue ("lpfSlope");
    auto lpfQ = parameters.getRawParameterValue ("lpfQ");
    auto filtersBypass = parameters.getRawParameterValue ("filtersBypass");

    auto ctrlCompThresh = parameters.getRawParameterValue ("ctrlCompThresh");
    auto ctrlCompAR = parameters.getRawParameterValue ("ctrlCompAR");
    auto ctrlCompBypass = parameters.getRawParameterValue ("ctrlCompBypass");

    auto styleCompAlgo = parameters.getRawParameterValue ("styleCompAlgo");
    auto styleCompIn = parameters.getRawParameterValue ("styleCompIn");
    auto styleCompMakeup = parameters.getRawParameterValue ("styleCompMakeup");
    auto styleCompMix = parameters.getRawParameterValue ("styleCompMix");
    auto styleCompBypass = parameters.getRawParameterValue ("styleCompBypass");

    auto consoleAlgo = parameters.getRawParameterValue ("consoleAlgo");
    auto consoleDrive = parameters.getRawParameterValue ("consoleDrive");
    auto consoleBypass = parameters.getRawParameterValue ("consoleBypass");

    auto outStageAlgo = parameters.getRawParameterValue ("outStageAlgo");
    auto outStageDrive = parameters.getRawParameterValue ("outStageDrive");
    auto outStageBypass = parameters.getRawParameterValue ("outStageBypass");

    auto outputGain = parameters.getRawParameterValue ("outputGain");
    auto volumeBypass = parameters.getRawParameterValue ("volumeBypass");

    // Channel Variation parameters
    auto channelVariationMode = parameters.getRawParameterValue ("channelVariationMode");
    auto channelPair = parameters.getRawParameterValue ("channelPair");

    // Update all sections (dual-mono)
    for (int ch = 0; ch < 2; ++ch)
    {
        // Determine which channel variation preset to use
        int variationIndex = -1;  // -1 = no variation (Off mode)

        if (channelVariationMode != nullptr && channelPair != nullptr)
        {
            int mode = static_cast<int> (*channelVariationMode);
            int pair = static_cast<int> (*channelPair);

            if (mode == 1)  // Stereo mode (L≠R)
            {
                variationIndex = pair * 2 + ch;  // pair 0: ch0→0, ch1→1; pair 1: ch0→2, ch1→3, etc.
            }
            else if (mode == 2)  // Mono mode (L=R, both use left channel preset)
            {
                variationIndex = pair * 2;  // Both channels use the left channel of the pair
            }
            // mode == 0 (Off): variationIndex stays -1
        }

        // Get channel variation preset (or use neutral if Off)
        const auto& cv = (variationIndex >= 0 && variationIndex < ChannelVariations::NUM_CHANNELS)
            ? ChannelVariations::presets[variationIndex]
            : ChannelVariationPreset{};  // Neutral preset (all zeros)
        // Section 1: Pre-Input
        if (preInputAlgo != nullptr)
            preInput[ch].setAlgorithm (static_cast<PreInputSection::Algorithm> (static_cast<int> (*preInputAlgo)));
        if (preInputDrive != nullptr)
            preInput[ch].setDrive (*preInputDrive);
        if (preInputBypass != nullptr)
            preInput[ch].setBypass (*preInputBypass > 0.5f);

        // Section 2: Filters
        if (hpfFreq != nullptr && hpfSlope != nullptr && hpfQ != nullptr)
        {
            FilterSection::Slope hpfSlopeEnum = (*hpfSlope < 0.5f)
                ? FilterSection::Slope_12dB
                : FilterSection::Slope_18dB;
            FilterSection::QMode hpfQEnum = (*hpfQ < 0.5f)
                ? FilterSection::Normal
                : FilterSection::Bump;

            // Apply channel variation freq offset
            float hpfFreqWithVariation = *hpfFreq + cv.hpfFreq;
            filters[ch].setHPF (hpfFreqWithVariation, hpfSlopeEnum, hpfQEnum);

            // Apply channel variation Q offset
            filters[ch].setHPFQOffset (cv.hpfQ);
        }

        if (lpfFreq != nullptr && lpfSlope != nullptr && lpfQ != nullptr)
        {
            FilterSection::Slope lpfSlopeEnum = (*lpfSlope < 0.5f)
                ? FilterSection::Slope_6dB
                : FilterSection::Slope_12dB;
            FilterSection::QMode lpfQEnum = (*lpfQ < 0.5f)
                ? FilterSection::Normal
                : FilterSection::Bump;

            // Apply channel variation freq offset
            float lpfFreqWithVariation = *lpfFreq + cv.lpfFreq;
            filters[ch].setLPF (lpfFreqWithVariation, lpfSlopeEnum, lpfQEnum);

            // Apply channel variation Q offset
            filters[ch].setLPFQOffset (cv.lpfQ);
        }

        if (filtersBypass != nullptr)
            filters[ch].setBypass (*filtersBypass > 0.5f);

        // Section 3: Control-Comp
        if (ctrlCompThresh != nullptr)
            controlComp[ch].setThreshold (*ctrlCompThresh);
        if (ctrlCompAR != nullptr)
        {
            ControlCompSection::ARMode arMode = (*ctrlCompAR < 0.5f)
                ? ControlCompSection::Fast
                : ControlCompSection::Normal;
            controlComp[ch].setARMode (arMode);
        }
        if (ctrlCompBypass != nullptr)
            controlComp[ch].setBypass (*ctrlCompBypass > 0.5f);

        // Section 4: EQ
        auto eqBass = parameters.getRawParameterValue ("eqBass");
        auto eqBassFreq = parameters.getRawParameterValue ("eqBassFreq");
        auto eqTreble = parameters.getRawParameterValue ("eqTreble");
        auto eqTrebleFreq = parameters.getRawParameterValue ("eqTrebleFreq");
        auto eqBell1Freq = parameters.getRawParameterValue ("eqBell1Freq");
        auto eqBell1Gain = parameters.getRawParameterValue ("eqBell1Gain");
        auto eqBell2Freq = parameters.getRawParameterValue ("eqBell2Freq");
        auto eqBell2Gain = parameters.getRawParameterValue ("eqBell2Gain");
        auto eqBypass = parameters.getRawParameterValue ("eqBypass");

        // Apply channel variations to Bass shelf
        if (eqBass != nullptr)
            eq[ch].setBassShelf (*eqBass + cv.eqBassGain);
        if (eqBassFreq != nullptr)
            eq[ch].setBassShelfFreq (*eqBassFreq + cv.eqBassFreq);

        // Apply channel variations to Treble shelf
        if (eqTreble != nullptr)
            eq[ch].setTrebleShelf (*eqTreble + cv.eqTrebleGain);
        if (eqTrebleFreq != nullptr)
            eq[ch].setTrebleShelfFreq (*eqTrebleFreq + cv.eqTrebleFreq);

        // Apply channel variations to Bell 1
        if (eqBell1Freq != nullptr && eqBell1Gain != nullptr)
            eq[ch].setBell1WithVariation (static_cast<int> (*eqBell1Freq), *eqBell1Gain,
                                          cv.eqBell1Freq, cv.eqBell1Gain, cv.eqBell1Q);

        // Apply channel variations to Bell 2
        if (eqBell2Freq != nullptr && eqBell2Gain != nullptr)
            eq[ch].setBell2WithVariation (static_cast<int> (*eqBell2Freq), *eqBell2Gain,
                                          cv.eqBell2Freq, cv.eqBell2Gain, cv.eqBell2Q);

        if (eqBypass != nullptr)
            eq[ch].setBypass (*eqBypass > 0.5f);

        // Section 5: Style-Comp
        if (styleCompAlgo != nullptr)
        {
            StyleCompSection::Algorithm styleAlgo = (*styleCompAlgo < 0.5f)
                ? StyleCompSection::Warm
                : StyleCompSection::Punch;
            styleComp[ch].setAlgorithm (styleAlgo);
        }
        if (styleCompIn != nullptr)
            styleComp[ch].setCompIn (*styleCompIn);
        if (styleCompMakeup != nullptr)
            styleComp[ch].setMakeup (*styleCompMakeup);
        if (styleCompMix != nullptr)
            styleComp[ch].setMix (*styleCompMix);
        if (styleCompBypass != nullptr)
            styleComp[ch].setBypass (*styleCompBypass > 0.5f);

        // Section 6: Console
        if (consoleAlgo != nullptr)
            console[ch].setAlgorithm (static_cast<ConsoleSection::Algorithm> (static_cast<int> (*consoleAlgo)));
        if (consoleDrive != nullptr)
            console[ch].setDrive (*consoleDrive + cv.consoleDrive);  // Apply channel variation
        if (consoleBypass != nullptr)
            console[ch].setBypass (*consoleBypass > 0.5f);

        // Section 7: OutStage
        if (outStageAlgo != nullptr)
            outStage[ch].setAlgorithm (static_cast<OutStageSection::Algorithm> (static_cast<int> (*outStageAlgo)));
        if (outStageDrive != nullptr)
            outStage[ch].setDrive (*outStageDrive);
        if (outStageBypass != nullptr)
            outStage[ch].setBypass (*outStageBypass > 0.5f);

        // Section 8: Volume
        if (outputGain != nullptr)
            volume[ch].setGain (*outputGain + cv.outputGain);  // Apply channel variation
        if (volumeBypass != nullptr)
            volume[ch].setBypass (*volumeBypass > 0.5f);
    }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout AnalogChannelAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ============================================================================
    // SECTION 1: Pre-Input
    // ============================================================================
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "preInputAlgo", "Pre-Input Algorithm",
        juce::StringArray { "Clean", "Pure", "Tape", "Tube" },
        1)); // Default: Pure

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "preInputDrive", "Pre-Input Drive",
        juce::NormalisableRange<float> (-18.0f, 18.0f, 0.1f),
        0.0f, "dB"));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "preInputBypass", "Pre-Input Bypass", false));

    // ============================================================================
    // SECTION 2: Filters
    // ============================================================================
    // HPF
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "hpfFreq", "HPF Frequency",
        juce::NormalisableRange<float> (20.0f, 6000.0f, 1.0f, 0.3f), // Skew for log scale
        20.0f, "Hz"));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "hpfSlope", "HPF Slope",
        juce::StringArray { "12 dB/oct", "18 dB/oct" },
        0)); // Default: 12 dB/oct

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "hpfQ", "HPF Q",
        juce::StringArray { "Normal", "Bump" },
        0)); // Default: Normal

    // LPF
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "lpfFreq", "LPF Frequency",
        juce::NormalisableRange<float> (300.0f, 24000.0f, 1.0f, 0.3f),
        24000.0f, "Hz"));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "lpfSlope", "LPF Slope",
        juce::StringArray { "6 dB/oct", "12 dB/oct" },
        0)); // Default: 6 dB/oct

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "lpfQ", "LPF Q",
        juce::StringArray { "Normal", "Bump" },
        0));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "filtersBypass", "Filters Bypass", false));

    // ============================================================================
    // SECTION 3: Control-Comp
    // ============================================================================
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "ctrlCompThresh", "Control-Comp Threshold",
        juce::NormalisableRange<float> (-30.0f, -0.1f, 0.1f),
        -10.0f, "dB"));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "ctrlCompAR", "Control-Comp A/R",
        juce::StringArray { "Fast", "Normal" },
        1)); // Default: Normal

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "ctrlCompBypass", "Control-Comp Bypass", false));

    // ============================================================================
    // SECTION 4: EQ
    // ============================================================================
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "eqBass", "EQ Bass",
        juce::NormalisableRange<float> (-15.0f, 15.0f, 0.1f),
        0.0f, "dB"));

    // Bass frequency with inverted mapping (knob left->right = 0->10, but freq 6500Hz->600Hz)
    juce::NormalisableRange<float> bassFreqRange (
        600.0f, 6500.0f,  // Normal range for internal processing
        [](float start, float end, float normalised) {
            // Convert 0->1 to 6500Hz->600Hz (inverted)
            return start + (end - start) * (1.0f - normalised);
        },
        [](float start, float end, float value) {
            // Convert Hz back to 0->1 (inverted)
            return 1.0f - (value - start) / (end - start);
        },
        [](float start, float end, float value) {
            // Snap function
            return value;
        });
    bassFreqRange.setSkewForCentre(2000.0f); // Logarithmic feel

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "eqBassFreq", "EQ Bass Mid Cut",
        bassFreqRange,
        6500.0f, "Hz")); // Default: 6.5kHz (displays as 0.0)

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "eqTreble", "EQ Treble",
        juce::NormalisableRange<float> (-15.0f, 15.0f, 0.1f),
        0.0f, "dB"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "eqTrebleFreq", "EQ Treble Mid Cut",
        juce::NormalisableRange<float> (3500.0f, 8200.0f, 1.0f, 0.3f), // Skew for log scale
        3500.0f, "Hz")); // Default: 3.5kHz (displays as 0.0)

    // Bell 1
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "eqBell1Freq", "EQ Bell 1 Frequency",
        juce::StringArray { "50", "100", "200", "300", "400", "500", "700", "900", "1.4k", "2.4k", "3.5k", "5k", "7.5k", "10k", "13k" },
        8)); // Default: 1.4kHz

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        "eqBell1Gain", "EQ Bell 1 Gain",
        -12, 20, 0, "dB"));  // Extended range to +20dB

    // Bell 2
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "eqBell2Freq", "EQ Bell 2 Frequency",
        juce::StringArray { "50", "100", "200", "300", "400", "500", "700", "900", "1.4k", "2.4k", "3.5k", "5k", "7.5k", "10k", "13k" },
        10)); // Default: 3.5kHz

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        "eqBell2Gain", "EQ Bell 2 Gain",
        -12, 20, 0, "dB"));  // Extended range to +20dB

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "eqBypass", "EQ Bypass", false));

    // ============================================================================
    // SECTION 5: Style-Comp
    // ============================================================================
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "styleCompAlgo", "Style-Comp Algorithm",
        juce::StringArray { "Warm", "Punch" },
        0)); // Default: Warm

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "styleCompIn", "Style-Comp IN",
        juce::NormalisableRange<float> (-18.0f, 60.0f, 0.1f),
        0.0f, "dB"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "styleCompMakeup", "Style-Comp Makeup",
        juce::NormalisableRange<float> (-6.0f, 24.0f, 0.1f),
        0.0f, "dB"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "styleCompMix", "Style-Comp Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        100.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "styleCompBypass", "Style-Comp Bypass", false));

    // ============================================================================
    // SECTION 6: Console
    // ============================================================================
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "consoleAlgo", "Console Algorithm",
        juce::StringArray { "Clean", "Pure", "Oxford", "Essex", "USA" },
        0)); // Default: Clean

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "consoleDrive", "Console Drive",
        juce::NormalisableRange<float> (-18.0f, 18.0f, 0.1f),
        0.0f, "dB"));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "consoleBypass", "Console Bypass", false));

    // ============================================================================
    // SECTION 7: Out Stage
    // ============================================================================
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "outStageAlgo", "Out Stage Algorithm",
        juce::StringArray { "Clean", "Pure", "Tape", "Tube", "Hard Clip", "Soft Clip" },
        0)); // Default: Clean

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "outStageDrive", "Out Stage Drive",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
        0.0f, "dB"));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "outStageBypass", "Out Stage Bypass", false));

    // ============================================================================
    // SECTION 8: Volume
    // ============================================================================
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "outputGain", "Output Gain",
        juce::NormalisableRange<float> (-60.0f, 12.0f, 0.1f),
        0.0f, "dB"));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "volumeBypass", "Volume Bypass", false));

    // ============================================================================
    // CHANNEL VARIATION - Analog Channel Modeling
    // ============================================================================
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "channelVariationMode", "Channel Variation Mode",
        juce::StringArray { "Off", "Stereo (L≠R)", "Mono (L=R)" },
        0)); // Default: Off

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        "channelPair", "Channel Pair",
        0, 23, // 0-23 = channels 1-48 (pairs: 1|2, 3|4, ..., 47|48)
        0)); // Default: pair 0 (channels 1|2)

    return { params.begin(), params.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogChannelAudioProcessor();
}
