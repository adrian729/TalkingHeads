/*
  ==============================================================================

	Class to store the raw parameter value

  ==============================================================================
*/

#include "ParameterObject.h"

//==============================================================================
ParameterObject::ParameterObject() : parameterType(ParameterType::Float) {}

ParameterObject::ParameterObject(juce::RangedAudioParameter* parameter, ParameterType parameterType) :
	parameter(parameter),
	parameterType(parameterType),
	smoothedVariable(SmoothingObject())
{
	// -- Update the inBound variables to initialize them
	updateInBoundVariable();
}

ParameterObject::ParameterObject(juce::RangedAudioParameter* parameter, SmoothingType smoothingType, double sampleRate, double rampLengthInSeconds) :
	parameter(parameter),
	parameterType(ParameterType::Float), // If we have smoothing it must be a continuous parameter (float). If smoothing is not needed, use the other constructor
	smoothedVariable(SmoothingObject(smoothingType, parameter->getNormalisableRange()))
{
	// -- Setup smoothed variable
	//smoothedVariable.reset(sampleRate, rampLengthInSeconds);

	// -- Update the inBound variables to initialize them
	updateInBoundVariable();

	// -- Set the smoothed current value to the updated inBound variable
	//smoothedVariable.setCurrentAndTargetValue(inBoundVariable);
}

ParameterObject::~ParameterObject()
{
	parameter = nullptr;
}

//==============================================================================
// -- Parameter getters
juce::RangedAudioParameter* ParameterObject::getParameter()
{
	return parameter;
}

ParameterType ParameterObject::getParameterType()
{
	return parameterType;
}

juce::AudioParameterBool* ParameterObject::getParameterBool()
{
	return (juce::AudioParameterBool*)parameter;
}

juce::AudioParameterChoice* ParameterObject::getParameterChoice()
{
	return (juce::AudioParameterChoice*)parameter;
}

juce::AudioParameterFloat* ParameterObject::getParameterFloat()
{
	return (juce::AudioParameterFloat*)parameter;
}

juce::AudioParameterInt* ParameterObject::getParameterInt()
{
	return (juce::AudioParameterInt*)parameter;
}

//==============================================================================
// -- InBound variable getters
float ParameterObject::getInBoundVariable()
{
	return inBoundVariable.load(std::memory_order_relaxed);
}

bool ParameterObject::getBoolValue()
{
	return static_cast<bool>(getInBoundVariable());
}

int ParameterObject::getChoiceIndex()
{
	return static_cast<int>(getInBoundVariable());
}

template <typename ChoiceType>
ChoiceType ParameterObject::getChoiceValue()
{
	return static_cast<ChoiceType>getChoiceIndex();
}

float ParameterObject::getFloatValue()
{
	return getInBoundVariable();
}

int ParameterObject::getIntValue()
{
	return static_cast<int>(getInBoundVariable());
}

//==============================================================================
// -- Smoothing
float ParameterObject::getSmoothedCurrentValue()
{
	return smoothedVariable.smoothingType != SmoothingType::NoSmoothing ? smoothedVariable.getCurrentValue() : getFloatValue();
}

float ParameterObject::getSmoothedValue()
{
	return smoothedVariable.smoothingType != SmoothingType::NoSmoothing ? smoothedVariable.getNextValue() : getFloatValue();
}

void ParameterObject::reset(double sampleRate, double rampLengthInSeconds)
{
	smoothedVariable.reset(sampleRate, rampLengthInSeconds);
}

void ParameterObject::setCurrentAndTargetValue(float newValue)
{
	smoothedVariable.setCurrentAndTargetValue(newValue);
}

void ParameterObject::setTargetValue(float newValue)
{
	smoothedVariable.setTargetValue(newValue);
}

//==============================================================================
void ParameterObject::updateInBoundVariable()
{
	// -- Update the inBound variable
	switch (parameterType)
	{
	case Bool:
		inBoundVariable = getParameterBool()->get();
		break;
	case Choice:
		inBoundVariable = getParameterChoice()->getIndex();
		break;
	case Int:
		inBoundVariable = getParameterInt()->get();
		break;
	case Float:
	default:
		inBoundVariable = getParameterFloat()->get();
		break;
	}

	// -- Set the smoothed value target to the inBound variable.
	//smoothedVariable.setTargetValue(inBoundVariable);
}

//==============================================================================
ParameterObject& ParameterObject::operator=(const ParameterObject& param)
{
	if (this == &param)
	{
		return *this;
	}

	parameter = param.parameter;
	parameterType = param.parameterType;
	smoothedVariable = param.smoothedVariable;
	inBoundVariable = param.inBoundVariable.load(std::memory_order_relaxed);

	return *this;
}