/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ParameterObject.h"

//==============================================================================
// --- MACROS enum
#define enumToInt(ENUM) static_cast<int>(ENUM)
#define intToEnum(INT, ENUM) static_cast<ENUM>(INT)
#define compareEnumToInt(ENUM, INT) (enumToInt(ENUM) == INT)
#define compareIntToEnum(INT, ENUM) (INT == enumToInt(ENUM))

//==============================================================================
// --- PARAMETER IDs
enum ControlID {
	gain,
	invertPhase,
	//==============================================================================
	countParams // value to keep track of the total number of parameters
};

struct PrepareParameterInfo {

	//==============================================================================
	// -- BOOL CONSTRUCTOR
	PrepareParameterInfo(
		ControlID id,
		const juce::String& name,
		bool defaultValue,
		const juce::String& label = juce::String()
	) :
		parameterID(juce::ParameterID { id, 1 }), // TODO: add versionHint setter or constructors
		name(name),
		parameterType(ParameterType::Bool),
		boolDefaultValue(defaultValue),
		label(label)
	{
	}
	// -- CHOICE CONSTRUCTOR
	PrepareParameterInfo(
		ControlID id,
		const juce::String& name,
		const juce::StringArray& choices,
		int defaultItemIndex,
		const juce::String& label = juce::String()
	) :
		parameterID(juce::ParameterID { id, 1 }), // TODO: add versionHint setter or constructors
		name(name),
		parameterType(ParameterType::Choice),
		choices(choices),
		defaultItemIndex(defaultItemIndex),
		label(label)
	{
	}
	// -- FLOAT CONSTRUCTOR
	PrepareParameterInfo(
		ControlID id,
		const juce::String& name,
		const juce::NormalisableRange<float>& floatRange,
		float defaultValue,
		SmoothingType smoothingType,
		const juce::String& label = juce::String()
	) :
		parameterID(juce::ParameterID { id, 1 }), // TODO: add versionHint setter or constructors
		name(name),
		parameterType(ParameterType::Float),
		floatRange(floatRange),
		floatDefaultValue(defaultValue),
		smoothingType(smoothingType),
		label(label)
	{
	}
	// -- INT CONSTRUCTOR
	PrepareParameterInfo(
		ControlID id,
		const juce::String& name,
		int minValue,
		int maxValue,
		int defaultValue,
		const juce::String& label = juce::String()
	) :
		parameterID(juce::ParameterID { id, 1 }), // TODO: add versionHint setter or constructors
		name(name),
		parameterType(ParameterType::Int),
		minValue(minValue),
		maxValue(maxValue),
		intDefaultValue(defaultValue),
		label(label)
	{
	}

	//==============================================================================
	// -- Basic parameter info
	juce::ParameterID parameterID;
	juce::String name;
	//==============================================================================
	// -- Extra parameter info
	juce::String label { juce::String() };

	//==============================================================================
	// -- Parameter type
	ParameterType parameterType{ ParameterType::Float };

	//==============================================================================
	// -- BOOL parameters
	bool boolDefaultValue{ false };
	// -- CHOICE parameters
	juce::StringArray choices{juce::StringArray()};
	int defaultItemIndex{ 0 };
	// -- FLOAT parameters
	juce::NormalisableRange<float> floatRange {juce::NormalisableRange<float>()};
	float floatDefaultValue{ 0.f };
	SmoothingType smoothingType{ SmoothingType::NoSmoothing };
	// -- INT parameters
	int minValue{ 0 };
	int maxValue{ 0 };
	int intDefaultValue{ 0 };


	//==============================================================================
	juce::String getParamID() const
	{
		return parameterID.getParamID();
	}
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
	const PrepareParameterInfo prepareParameterInfoArr[ControlID::countParams] = {
		PrepareParameterInfo(
			ControlID::gain,
			"Gain",
			juce::NormalisableRange<float>(-24.f, 24.f, 0.01f),
			0.f,
			SmoothingType::Linear,
			"dB"
		),
		PrepareParameterInfo(
			ControlID::invertPhase,
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
	juce::NormalisableRange<float> range;
	juce::LinearSmoothedValue<float> gainSmoothedValue;
	float gain;
	int invertPhase;

	//==============================================================================
	void initPluginParameters(double sampleRate);
	void initPluginMemberVariables(double sampleRate);

	//==============================================================================
	void preProcessBlock();
	void postProcessBlock();

	void syncInBoundVariables();
	bool postUpdatePluginParameter(ControlID parameterID);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TemplateDSPPluginAudioProcessor)
};
