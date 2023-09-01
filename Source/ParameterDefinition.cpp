/*
  ==============================================================================

	ParameterDefinition.cpp
	Created: 30 Aug 2023 12:20:23pm
	Author:  Brutus729

  ==============================================================================
*/

#include "ParameterDefinition.h"

//==============================================================================
// -- CONSTRUCTORS
// -- DEFAULT CONSTRUCTOR
ParameterDefinition::ParameterDefinition() :
	parameterID(juce::ParameterID()),
	name(juce::String()),
	parameterType(ParameterType::Float),
	floatRange(juce::NormalisableRange<float>()),
	defaultValue(0.f),
	suffixLabel(""),
	smoothingType(SmoothingType::NoSmoothing)
{
}
// -- BOOL
ParameterDefinition::ParameterDefinition(
	int id,
	int versionHint,
	const juce::String& name,
	bool defaultValue,
	const juce::String& suffixLabel,
	SmoothingType smoothingType
) :
	parameterID(juce::ParameterID{ id, versionHint }),
	name(name),
	parameterType(ParameterType::Bool),
	defaultValue(static_cast<float>(defaultValue)),
	suffixLabel(suffixLabel),
	smoothingType(smoothingType)
{
}
// -- CHOICE CONSTRUCTOR
ParameterDefinition::ParameterDefinition(
	int id,
	int versionHint,
	const juce::String& name,
	const juce::StringArray& choices,
	int defaultItemIndex,
	const juce::String& suffixLabel,
	SmoothingType smoothingType
) :
	parameterID(juce::ParameterID{ id, versionHint }),
	name(name),
	parameterType(ParameterType::Choice),
	choices(choices),
	defaultValue(static_cast<float>(defaultItemIndex)),
	suffixLabel(suffixLabel),
	smoothingType(smoothingType)
{
}
// -- FLOAT CONSTRUCTOR
ParameterDefinition::ParameterDefinition(
	int id,
	int versionHint,
	const juce::String& name,
	const juce::NormalisableRange<float>& floatRange,
	float defaultValue,
	const juce::String& suffixLabel,
	SmoothingType smoothingType
) :
	parameterID(juce::ParameterID{ id, versionHint }),
	name(name),
	parameterType(ParameterType::Float),
	floatRange(floatRange),
	defaultValue(defaultValue),
	suffixLabel(suffixLabel),
	smoothingType(smoothingType)
{
}
// -- INT CONSTRUCTOR
ParameterDefinition::ParameterDefinition(
	int id,
	int versionHint,
	const juce::String& name,
	int minValue,
	int maxValue,
	int defaultValue,
	const juce::String& suffixLabel,
	SmoothingType smoothingType
) :
	parameterID(juce::ParameterID{ id, versionHint }),
	name(name),
	parameterType(ParameterType::Int),
	minValue(minValue),
	maxValue(maxValue),
	defaultValue(static_cast<float>(defaultValue)),
	suffixLabel(suffixLabel),
	smoothingType(smoothingType)
{
}

ParameterDefinition::~ParameterDefinition()
{
}

//==============================================================================
// -- Parameter getters
juce::ParameterID ParameterDefinition::getParameterID() const
{
	return parameterID;
}

juce::String ParameterDefinition::getParamID() const
{
	return parameterID.getParamID();
}

int ParameterDefinition::getVersionHint() const
{
	return parameterID.getVersionHint();
}

juce::String ParameterDefinition::getName() const
{
	return name;
}

ParameterType ParameterDefinition::getParameterType() const
{
	return parameterType;
}


//==============================================================================
bool ParameterDefinition::getBoolDefaultValue() const
{
	return static_cast<bool>(defaultValue);
}

juce::StringArray ParameterDefinition::getChoices() const
{
	return choices;
}

int ParameterDefinition::getChoiceDefaultIndex() const
{
	return static_cast<int>(defaultValue);
}

float ParameterDefinition::getFloatDefaultValue() const
{
	return defaultValue;
}

juce::NormalisableRange<float> ParameterDefinition::getFloatRange() const
{
	return floatRange;
}

SmoothingType ParameterDefinition::getSmoothingType() const
{
	return smoothingType;
}

int ParameterDefinition::getIntDefaultValue() const
{
	return static_cast<int>(defaultValue);
}

int ParameterDefinition::getIntMinValue() const
{
	return minValue;
}

int ParameterDefinition::getIntMaxValue() const
{
	return maxValue;
}

//==============================================================================
juce::String ParameterDefinition::getSuffixLabel() const
{
	return suffixLabel;
}

//==============================================================================
std::unique_ptr<juce::RangedAudioParameter> ParameterDefinition::createParameter() const
{
	switch (parameterType)
	{
	case ParameterType::Bool:
		return std::make_unique<juce::AudioParameterBool>(
			parameterID,
			name,
			getBoolDefaultValue(),
			juce::AudioParameterBoolAttributes().withLabel(suffixLabel)
		);
	case ParameterType::Choice:
		return std::make_unique<juce::AudioParameterChoice>(
			parameterID,
			name,
			choices,
			getChoiceDefaultIndex(),
			juce::AudioParameterChoiceAttributes().withLabel(suffixLabel)
		);
	case ParameterType::Float:
		return std::make_unique<juce::AudioParameterFloat>(
			parameterID,
			name,
			floatRange,
			getFloatDefaultValue(),
			juce::AudioParameterFloatAttributes().withLabel(suffixLabel)
		);
	case ParameterType::Int:
		return std::make_unique<juce::AudioParameterInt>(
			parameterID,
			name,
			minValue,
			maxValue,
			getIntDefaultValue(),
			juce::AudioParameterIntAttributes().withLabel(suffixLabel)
		);
	default:
		jassertfalse;
		return nullptr;
	}
}
