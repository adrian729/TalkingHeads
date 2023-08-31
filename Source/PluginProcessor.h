/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "ParameterObject.h"
#include "ParameterDefinition.h"

//==============================================================================
// --- PARAMETER IDs
enum ControlID {
	bypass,
	blend,
	gain,
	//==============================================================================
	countParams // value to keep track of the total number of parameters
};

//==============================================================================
/**
*/
class TalkingHeadsPluginAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	, public juce::AudioProcessorARAExtension
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

private:
	//==============================================================================
	// --- Object parameters management and information

	// --- Parameters definitions - make sure each item in the array matches the order of the enum
	const std::array<ParameterDefinition, ControlID::countParams> parameterDefinitions = createParameterDefinitions();

	// --- Parameters state APVTS
	const juce::String PARAMETERS_APVTS_ID = "ParametersAPVTS";
	juce::AudioProcessorValueTreeState parametersAPVTS;

	// --- Object parameters and inBound variables
	ParameterObject pluginProcessorParameters[ControlID::countParams];

	//==============================================================================
	// --- Object member variables

	// -- General process
	float bypass; // -- using a float to smooth the bypass transition
	float blend;
	float gain;

	//==============================================================================
	const std::array<ParameterDefinition, ControlID::countParams> createParameterDefinitions();
	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	void initPluginParameters();
	void initPluginMemberVariables(double sampleRate);

	//==============================================================================
	void preProcessBlock();
	void postProcessBlock();

	void syncInBoundVariables();
	bool postUpdatePluginParameter(ControlID parameterID);

	//==============================================================================
	float blendValues(float dry, float wet, float blend);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TalkingHeadsPluginAudioProcessor)
};
