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

	// -- General parameters
	ControlID bypassID{ ControlID::bypass };
	ControlID blendID{ ControlID::blend };
	ControlID preGainID{ ControlID::preGain };

	// -- Phaser
	ControlID phaserBypassID{ ControlID::phaserBypass };
	ControlID phaserRateID{ ControlID::phaserRate };
	ControlID phaserDepthID{ ControlID::phaserDepth };
	ControlID phaserCentreFrequencyID{ ControlID::phaserCentreFrequency };
	ControlID phaserFeedbackID{ ControlID::phaserFeedback };
	ControlID phaserMixID{ ControlID::phaserMix };


	// --- Parameters state APVTS
	const juce::String PARAMETERS_APVTS_ID = "ParametersAPVTS";
	juce::AudioProcessorValueTreeState parametersAPVTS;

	// --- Object parameters and inBound variables
	std::array<ParameterObject, ControlID::countParams> pluginProcessorParameters;

	//==============================================================================
	// --- Object member variables

	// --- stage 0: General -- Bypass ALL // Blend (dry/wet)
	float bypass{ 0.f }; // -- using a float to smooth the bypass transition
	float blend{ 0.f };

	juce::dsp::DryWetMixer<float> blendMixer;
	juce::AudioBuffer<float> blendMixerBuffer; // -- buffer to replicate mono signal to all channels for the blend mixer

	// -- Mono Stages
	enum monoChainIndex
	{
		preGainIndex,
		multiEQIndex,
		multiCompIndex
	};
	juce::dsp::ProcessorChain<
		juce::dsp::Gain<float>,
		MultiBandEQ,
		MultiBandCompressor
	> monoChain;

	// -- Multi channel stages
	enum multiChannelChainIndex
	{
		phaserIndex
	};
	juce::dsp::ProcessorChain<
		juce::dsp::Phaser<float>
	> multiChannelChain;

	bool phaserBypass{ false };

	//==============================================================================
	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	void initPluginParameters();
	void initPluginMemberVariables(double sampleRate, int samplesPerBlock);

	void initBlendMixer(double sampleRate, int samplesPerBlock);
	void initMonoMultiEQ();
	void initMonoMultiCompressor();

	//==============================================================================
	void preProcessBlock();
	void postProcessBlock();

	void syncInBoundVariables();
	void postUpdatePluginParameters();

	float getLatency();

	//==============================================================================
	float blendValues(float dry, float wet, float blend);
	float getIndexInterpolationFactor(int originSize, int newSize);

	//==============================================================================
	juce::dsp::AudioBlock<float> createDryBlock(juce::AudioBuffer<float>& buffer, int numChannels, int numSamples);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TalkingHeadsPluginAudioProcessor)
};
