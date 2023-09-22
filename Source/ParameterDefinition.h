/*
  ==============================================================================

	ParameterDefinition.h
	Created: 30 Aug 2023 12:20:23pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"

//==============================================================================
class ParameterDefinition
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	// -- DEFAULT CONSTRUCTOR
	ParameterDefinition();
	// -- BOOL
	ParameterDefinition(
		ControlID controlID,
		juce::String id,
		int versionHint,
		const juce::String& name,
		bool defaultValue = false,
		const juce::String& suffixLabel = "",
		SmoothingType smoothingType = SmoothingType::NoSmoothing
	);
	// -- CHOICE CONSTRUCTOR
	ParameterDefinition(
		ControlID controlID,
		juce::String id,
		int versionHint,
		const juce::String& name,
		const juce::StringArray& choices,
		int defaultItemIndex = 0,
		const juce::String& suffixLabel = "",
		SmoothingType smoothingType = SmoothingType::NoSmoothing
	);
	// -- FLOAT CONSTRUCTOR
	ParameterDefinition(
		ControlID controlID,
		juce::String id,
		int versionHint,
		const juce::String& name,
		const juce::NormalisableRange<float>& floatRange,
		float defaultValue = 0.f,
		const juce::String& suffixLabel = "",
		SmoothingType smoothingType = SmoothingType::NoSmoothing
	);
	// -- INT CONSTRUCTOR
	ParameterDefinition(
		ControlID controlID,
		juce::String id,
		int versionHint,
		const juce::String& name,
		int minValue,
		int maxValue,
		int defaultValue = 0,
		const juce::String& suffixLabel = "",
		SmoothingType smoothingType = SmoothingType::NoSmoothing
	);

	~ParameterDefinition();

	//==============================================================================
	// -- Parameter getters
	ControlID getControlID() const;

	juce::ParameterID getParameterID() const;

	juce::String getParamID() const;

	int getVersionHint() const;

	juce::String getName() const;

	ParameterType getParameterType() const;

	//==============================================================================
	bool getBoolDefaultValue() const;

	juce::StringArray getChoices() const;

	int getChoiceDefaultIndex() const;

	float getFloatDefaultValue() const;

	juce::NormalisableRange<float> getFloatRange() const;

	SmoothingType getSmoothingType() const;

	int getIntDefaultValue() const;

	int getIntMinValue() const;

	int getIntMaxValue() const;

	//==============================================================================
	juce::String getSuffixLabel() const;

	//==============================================================================
	std::unique_ptr<juce::RangedAudioParameter> createParameter() const;

private:
	//==============================================================================
	// -- Basic parameter info
	ControlID controlID;
	juce::ParameterID parameterID;
	juce::String name;

	//==============================================================================
	// -- Parameter information
	ParameterType parameterType{ ParameterType::Float };
	float defaultValue; // will be cast to the appropriate type
	// -- CHOICE parameters
	juce::StringArray choices{ juce::StringArray() };
	// -- FLOAT parameters
	juce::NormalisableRange<float> floatRange{ juce::NormalisableRange<float>() };
	SmoothingType smoothingType{ SmoothingType::NoSmoothing };
	// -- INT parameters
	int minValue{ 0 };
	int maxValue{ 0 };

	//==============================================================================
	// -- Extra parameter info
	juce::String suffixLabel;
};
