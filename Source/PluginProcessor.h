/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ParameterObject.h"
#include "parameterTypes.h"
#include "ParameterDefinition.h"

//==============================================================================
// --- PARAMETER IDs
enum ControlID {
	gain,
	invertPhase,
	//==============================================================================
	countParams // value to keep track of the total number of parameters
};

//==============================================================================
/**
*/
class TemplateDSPPluginAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	, public juce::AudioProcessorARAExtension
#endif
{
public:
	//==============================================================================
	TemplateDSPPluginAudioProcessor();
	~TemplateDSPPluginAudioProcessor() override;

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
	// --- Object parameters helper variables and structures
	const juce::String PARAMETERS_APVTS_ID = "ParametersAPVTS";

	// Make sure this each param position matches the ControlID enum in PluginProcessor.h
	const ParameterDefinition parameterDefinitions[ControlID::countParams] = {
		ParameterDefinition(
			ControlID::gain,
			1,
			"Gain",
			juce::NormalisableRange<float>(-24.f, 24.f, 0.01f),
			0.f,
			SmoothingType::Linear,
			"dB"
		),
		ParameterDefinition(
			ControlID::invertPhase,
			1,
			"Invert Phase",
			false
		)
	};

	//==============================================================================
	// --- Object parameters state
	juce::AudioProcessorValueTreeState parametersAPVTS;

	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	//==============================================================================
	// --- Object parameters and inBound variables
	ParameterObject pluginProcessorParameters[ControlID::countParams];

	//==============================================================================
	// --- Object member variables
	float gain;
	int invertPhase;

	//==============================================================================
	void initPluginParameters();
	void initPluginMemberVariables(double sampleRate);

	//==============================================================================
	void preProcessBlock();
	void postProcessBlock();

	void syncInBoundVariables();
	bool postUpdatePluginParameter(ControlID parameterID);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TemplateDSPPluginAudioProcessor)
};
