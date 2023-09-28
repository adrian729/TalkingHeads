/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>
#include "parameterTypes.h"
#include "PluginStateManager.h"
#include "ParameterObject.h"
#include "ParameterDefinition.h"
#include "MultiBandEQ.h"
#include "MultiBandCompressor.h"

//==============================================================================
/**
 */
class TalkingHeadsPluginAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	,
	public juce::AudioProcessorARAExtension
#endif
{
public:
	//==============================================================================
	TalkingHeadsPluginAudioProcessor();
	~TalkingHeadsPluginAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	//==============================================================================
	void reset() override;

private:
	//==============================================================================
	// --- Object parameters management and information

	// --- Parameters definitions
	PluginStateManager pluginStateManager{};
	std::array<ParameterDefinition, ControlID::countParams> parameterDefinitions = pluginStateManager.getParameterDefinitions();

	const std::set<ControlID> generalControlIDs = {
		ControlID::bypass,
		ControlID::blend,
		ControlID::preGain
	};

	// --- Parameters state APVTS
	const juce::String PARAMETERS_APVTS_ID = "ParametersAPVTS";
	juce::AudioProcessorValueTreeState parametersAPVTS;

	// --- Object parameters and inBound variables
	std::array<ParameterObject, ControlID::countParams> pluginProcessorParameters;

	//==============================================================================
	// --- Object member variables

	// --- stage 0: General -- Bypass ALL // Blend (dry/wet)
	float bypass; // -- using a float to smooth the bypass transition
	float blend;

	juce::dsp::Gain<float> preGainProcessor;
	juce::dsp::DryWetMixer<float> blendMixer;
	juce::AudioBuffer<float> blendMixerBuffer; // -- buffer to replicate mono signal to all channels for the blend mixer

	// -- stage 1 -- HPF, LPF, 3 Band EQ
	MultiBandEQ firstStageProcessor{ parameterDefinitions, pluginProcessorParameters };
	// -- stage 2 -- 3 Band Compressor
	MultiBandCompressor secondStageProcessor;

	//==============================================================================
	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	void initPluginParameters();
	void initPluginMemberVariables(double sampleRate, int samplesPerBlock);

	//==============================================================================
	void preProcessBlock();
	void postProcessBlock();

	void syncInBoundVariables();
	bool postUpdatePluginParameter(ControlID parameterID);

	float getLatency();

	//==============================================================================
	float blendValues(float dry, float wet, float blend);
	float getIndexInterpolationFactor(int originSize, int newSize);

	//==============================================================================
	juce::dsp::AudioBlock<float> createDryBlock(juce::AudioBuffer<float>& buffer, int numChannels, int numSamples);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TalkingHeadsPluginAudioProcessor)
};
