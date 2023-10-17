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
	const juce::String APVTS_ID = "ParametersAPVTS";
	std::shared_ptr<PluginStateManager> stateManager;

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

	//==============================================================================
	// --- Object member variables

	// --- stage 0: General -- Bypass ALL // Blend (dry/wet)
	float bypass{ 0.f }; // -- using a float to smooth the bypass transition
	float blend{ 0.f };
	juce::dsp::DryWetMixer<float> blendMixer;
	juce::AudioBuffer<float> blendMixerBuffer; // -- buffer to replicate mono signal to all channels for the blend mixer

	// TODO: change setups for processors and add it to their constructors
	// -- Mono Stages

	// -- Pre Gain
	float preGainGain{ 0.f };
	juce::dsp::Gain<float> preGain;

	// -- Multi Band EQ
	float multiBandEQSampleRate{ 0.f };
	MultiBandEQ multiBandEQ;

	// -- Multi Band Compressor
	MultiBandCompressor multiBandCompressor;

	// -- Multi channel stages

	// -- Phaser
	float phaserBypass{ 0.f };
	bool isPhaserBypassed{ false };
	float phaserRate{ 0.f };
	float phaserDepth{ 0.f };
	float phaserCentreFrequency{ 0.f };
	float phaserFeedback{ 0.f };
	float phaserMix{ 0.f };
	juce::dsp::Phaser<float> phaser;

	//==============================================================================
	void initBlendMixer(double sampleRate, int samplesPerBlock);
	void initMultiBandEQ(const juce::dsp::ProcessSpec& spec);
	void initPhaser(const juce::dsp::ProcessSpec& spec);

	//==============================================================================
	void preProcessBlock();
	void postProcessBlock();

	void postUpdatePluginParameters();
	void postUpdatePhaserParameters();
	//==============================================================================
	float getLatency();

	//==============================================================================
	juce::dsp::AudioBlock<float> createDryBlock(juce::AudioBuffer<float>& buffer, int numChannels, int numSamples);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TalkingHeadsPluginAudioProcessor)
};
