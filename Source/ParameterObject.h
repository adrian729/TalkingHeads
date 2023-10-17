/*
  ==============================================================================

	ParametersObject.h
	Parameter information for the plugin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"

enum class ParameterType
{
	Bool,
	Choice,
	Float,
	Int
};

class ParameterObject
{
public:
	//==============================================================================
	// -- Constructors & Destructors
	ParameterObject();
	ParameterObject(
		juce::RangedAudioParameter* parameter,
		ParameterType parameterType = ParameterType::Float
	);
	~ParameterObject();

	//==============================================================================
	ParameterObject(const ParameterObject&); // copy constructor
	ParameterObject(ParameterObject&&) noexcept; // move constructor

	ParameterObject& operator=(const ParameterObject& other); // copy assignment
	ParameterObject& operator=(ParameterObject&& other) noexcept; // move assignment

	//==============================================================================
	// -- Parameter
	ParameterType getParameterType();
	juce::NormalisableRange<float> getParameterRange();

	juce::RangedAudioParameter* getParameter();
	juce::AudioParameterBool* getParameterBool();
	juce::AudioParameterChoice* getParameterChoice();
	juce::AudioParameterFloat* getParameterFloat();
	juce::AudioParameterInt* getParameterInt();

	//==============================================================================
	// -- InBound variable
	float getInBoundVariable();

	bool getBoolValue();
	int getChoiceIndex();
	template <typename ChoiceType>
	ChoiceType getChoiceValue();
	float getFloatValue();
	int getIntValue();

	template <typename T>
	T getParameter();

	//==============================================================================
	float updateInBoundVariable();

private:
	//==============================================================================
	// -- Parameter data
	ParameterType parameterType;
	juce::RangedAudioParameter* parameter;
	//==============================================================================
	// -- InBound value
	std::atomic<float> inBoundVariable;
};
