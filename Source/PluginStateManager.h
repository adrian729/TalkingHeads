/*
  ==============================================================================

	This file contains the data structures and objects to define the plugin's parameter structure.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <map>
#include <array>
#include <memory>
#include "parameterTypes.h"
#include "ParameterObject.h"

//==============================================================================
class PluginStateManager
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	PluginStateManager(juce::AudioProcessor& processorToConnectTo, juce::UndoManager* undoManagerToUse, const juce::Identifier& valueTreeType);
	~PluginStateManager();

	//==============================================================================
	PluginStateManager(const PluginStateManager&); // copy constructor
	PluginStateManager(PluginStateManager&&) noexcept; // move constructor

	PluginStateManager& operator=(const PluginStateManager& other); // copy assignment
	PluginStateManager& operator=(PluginStateManager&& other) noexcept; // move assignment

	//==============================================================================
	// --- APVTS
	juce::AudioProcessorValueTreeState* getAPVTS();

	// -- Parameters
	void attachParameters(const juce::AudioProcessorValueTreeState& apvts);
	std::shared_ptr<ParameterObject> getParameterObject(ControlID controlID);
	juce::RangedAudioParameter* getParameter(ControlID controlID);

	// -- Inbound Values
	bool getBoolValue(ControlID controlID);
	int getChoiceIndex(ControlID controlID);
	template <typename ChoiceType>
	ChoiceType getChoiceValue(ControlID controlID);
	float getFloatValue(ControlID controlID);
	int getIntValue(ControlID controlID);

	// -- Smoothing
	void initSmoothedValues(double sampleRate);
	void resetSmoothedValues(double sampleRate);
	void reset(ControlID controlID, double sampleRate, double rampLengthInSeconds = 0.005f);

	float getCurrentValue(ControlID controlID);
	float getNextValue(ControlID controlID);

	void setCurrentAndTargetValue(ControlID controlID, float newValue);
	void setTargetValue(ControlID controlID, float newValue);

	//==============================================================================
	// -- PARAMETERS CREATION
	//==============================================================================
	// -- BOOL CONSTRUCTOR
	void addParam(
		juce::AudioProcessorValueTreeState::ParameterLayout& layout,
		ControlID controlID,
		juce::String id,
		int versionHint,
		const juce::String& name,
		bool defaultValue = false,
		const juce::String& suffixLabel = "",
		SmoothingType smoothingType = SmoothingType::NoSmoothing,
		double rampLengthInSeconds = 0.005f
	);
	// -- CHOICE CONSTRUCTOR
	void addParam(
		juce::AudioProcessorValueTreeState::ParameterLayout& layout,
		ControlID controlID,
		juce::String id,
		int versionHint,
		const juce::String& name,
		const juce::StringArray& choices,
		int defaultItemIndex = 0,
		const juce::String& suffixLabel = "",
		SmoothingType smoothingType = SmoothingType::NoSmoothing,
		double rampLengthInSeconds = 0.005f
	);
	// -- FLOAT CONSTRUCTOR
	void addParam(
		juce::AudioProcessorValueTreeState::ParameterLayout& layout,
		ControlID controlID,
		juce::String id,
		int versionHint,
		const juce::String& name,
		const juce::NormalisableRange<float>& floatRange,
		float defaultValue = 0.f,
		const juce::String& suffixLabel = "",
		SmoothingType smoothingType = SmoothingType::NoSmoothing,
		double rampLengthInSeconds = 0.005f
	);
	// -- INT CONSTRUCTOR
	void addParam(
		juce::AudioProcessorValueTreeState::ParameterLayout& layout,
		ControlID controlID,
		juce::String id,
		int versionHint,
		const juce::String& name,
		int minValue,
		int maxValue,
		int defaultValue = 0,
		const juce::String& suffixLabel = "",
		SmoothingType smoothingType = SmoothingType::NoSmoothing,
		double rampLengthInSeconds = 0.005f
	);

	//==============================================================================
	void syncInBoundVariables();
private:
	//==============================================================================
	// --- Parameters
	std::array<juce::String, ControlID::countParams> parameterIDs;
	std::array<ParameterType, ControlID::countParams> parameterTypes;
	// --- Parameter Objects holds the actual parameters and its inbound values
	std::array<std::shared_ptr<ParameterObject>, ControlID::countParams> parameters;

	//==============================================================================
	// -- Smoothing variables
	using Multiplicative = juce::ValueSmoothingTypes::Multiplicative;

	template <typename T>
	struct SmoothedValue
	{
		SmoothedValue(ControlID controlID, T smoothedValue, double rampLengthInSeconds) :
			controlID(controlID),
			smoothedValue(smoothedValue),
			rampLengthInSeconds(rampLengthInSeconds)
		{
		}

		ControlID controlID;
		T smoothedValue;
		double rampLengthInSeconds;
	};

	// -- (SmoothingType, idx) -> smoothingTypeArray[idx] = smoothedValue
	std::array<SmoothingType, ControlID::countParams> smoothingTypes;
	std::array<int, ControlID::countParams> smoothingValueIndexes;
	// -- smoothed values
	std::vector <SmoothedValue<juce::LinearSmoothedValue<float>>> linearSmoothedValues;
	std::vector <SmoothedValue<juce::SmoothedValue<float, Multiplicative>>> multiplicativeSmoothedValues;

	// --- Parameters APVTS
	const juce::String PARAMETERS_APVTS_ID = "ParametersAPVTS";
	juce::AudioProcessorValueTreeState* apvts_ptr;

	//==============================================================================
	// -- PRIVATE METHODS
	//==============================================================================
	void updateSmoothingValue(ControlID controlID, float lastValue, float newValue);

	//==============================================================================
	template<typename T>
	void addSmoothedValue(
		ControlID controlID,
		SmoothingType smoothingType,
		T initialValue,
		double rampLengthInSeconds
	);

	//==============================================================================
	float safeMultiplicativeValue(float value, float smallestValue = 0.0001f);
	float safeDenormalizableValue(float value);

	//==============================================================================
	juce::AudioProcessorValueTreeState::ParameterLayout initParamsAndCreateLayout();
};
