/*
  ==============================================================================

	Class to store the raw parameter value

  ==============================================================================
*/

#include "ParameterObject.h"

//==============================================================================
ParameterObject::ParameterObject() : 
	parameter(nullptr),
	parameterType(ParameterType::Float),
	smoothingType(SmoothingType::NoSmoothing)
{
}

ParameterObject::ParameterObject(juce::RangedAudioParameter* parameter, ParameterType parameterType) :
	parameter(parameter),
	parameterType(parameterType),
	smoothingType(SmoothingType::NoSmoothing)
{
	// -- Update the inBound variables to initialize them
	updateInBoundVariable();
}

ParameterObject::ParameterObject(juce::RangedAudioParameter* parameter, SmoothingType smoothingType) :
	parameter(parameter),
	parameterType(ParameterType::Float), // If we have smoothing it must be a continuous parameter (float). If smoothing is not needed, use the other constructor
	smoothingType(smoothingType)
{
	// -- Update the inBound variables to initialize them
	updateInBoundVariable();
}

ParameterObject::~ParameterObject()
{
	parameter = nullptr;
}

//==============================================================================
// -- Parameter getters
ParameterType ParameterObject::getParameterType()
{
	return parameterType;
}

juce::NormalisableRange<float>  ParameterObject::getParameterRange()
{
	return parameter->getNormalisableRange();
}

juce::RangedAudioParameter* ParameterObject::getParameter()
{
	return parameter;
}

template <typename T>
T ParameterObject::getParameter()
{
	T t_parameter = dynamic_cast<T>(parameter);
	jassert(t_parameter != nullptr);
	return t_parameter;
}

juce::AudioParameterBool* ParameterObject::getParameterBool()
{
	return getParameter<juce::AudioParameterBool*>();
}

juce::AudioParameterChoice* ParameterObject::getParameterChoice()
{
	return getParameter<juce::AudioParameterChoice*>();
}

juce::AudioParameterFloat* ParameterObject::getParameterFloat()
{
	return getParameter<juce::AudioParameterFloat*>();
}

juce::AudioParameterInt* ParameterObject::getParameterInt()
{
	return getParameter<juce::AudioParameterInt*>();
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
	return static_cast<ChoiceType>(getChoiceIndex());
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
void ParameterObject::initSmoothing(double sampleRate, double rampLengthInSeconds)
{
	reset(sampleRate, rampLengthInSeconds);
	setCurrentAndTargetValue(getFloatValue());
}

float ParameterObject::getCurrentValue()
{
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return getFloatValue();
	}

	float normalizedValue = 0.f;
	switch (smoothingType)
	{
	case SmoothingType::Linear:
		normalizedValue = linearSmoothedValue.getCurrentValue();
		break;
	case SmoothingType::Multiplicative:
		normalizedValue = multiplicativeSmoothedValue.getCurrentValue();
		break;
	}
	// Denormalize value to the selected range
	return getParameterRange().convertFrom0to1(normalizedValue);
}

float ParameterObject::getNextValue()
{
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return getFloatValue();
	}

	float normalizedValue = 0.f;
	for (int i = 0; i < 100; ++i)
	{
		switch (smoothingType)
		{
		case Linear:
			normalizedValue = linearSmoothedValue.getNextValue();
			break;
		case Multiplicative:
			normalizedValue = multiplicativeSmoothedValue.getNextValue();
			break;
		}
	}
	// Denormalize value to the selected range
	return getParameterRange().convertFrom0to1(normalizedValue);
}

void ParameterObject::setCurrentAndTargetValue(float newValue)
{
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return;
	}

	// Need to normalize the inBound variable to the range [0, 1] first
	float normalizedNewValue = getParameterRange().convertTo0to1(newValue);
	switch (smoothingType)
	{
	case SmoothingType::Linear:
		linearSmoothedValue.setCurrentAndTargetValue(normalizedNewValue);
		break;
	case SmoothingType::Multiplicative:
		multiplicativeSmoothedValue.setCurrentAndTargetValue(safeMultiplicativeValue(normalizedNewValue));
		break;
	}
}

void ParameterObject::setTargetValue(float newValue)
{
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return;
	}

	// Need to normalize the inBound variable to the range [0, 1] first
	float normalizedNewValue = getParameterRange().convertTo0to1(newValue);
	switch (smoothingType)
	{
	case SmoothingType::Linear:
		linearSmoothedValue.setTargetValue(normalizedNewValue);
		break;
	case SmoothingType::Multiplicative:
		multiplicativeSmoothedValue.setTargetValue(safeMultiplicativeValue(normalizedNewValue));
		break;
	}
}

void ParameterObject::reset(double sampleRate, double rampLengthInSeconds)
{
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return;
	}

	switch (smoothingType)
	{
	case SmoothingType::Linear:
		linearSmoothedValue.reset(sampleRate, 0.5f);
		break;
	case SmoothingType::Multiplicative:
		multiplicativeSmoothedValue.reset(sampleRate, rampLengthInSeconds);
		break;
	}
}

//==============================================================================
void ParameterObject::updateInBoundVariable()
{
	// -- Update the inBound variable
	switch (parameterType)
	{
	case Bool:
		inBoundVariable = static_cast<float>(getParameterBool()->get());
		break;
	case Choice:
		inBoundVariable = static_cast<float>(getParameterChoice()->getIndex());
		break;
	case Int:
		inBoundVariable = static_cast<float>(getParameterInt()->get());
		break;
	case Float:
		inBoundVariable = getParameterFloat()->get();
	}

	// -- Set the smoothed value target to the inBound variable.
	setTargetValue(inBoundVariable);
}

//==============================================================================
float  ParameterObject::safeMultiplicativeValue(float value, float smallestValue)
{
	return juce::approximatelyEqual(value, 0.f) ? smallestValue : value;
}

//==============================================================================
ParameterObject& ParameterObject::operator=(const ParameterObject& param)
{
	if (this == &param)
	{
		return *this;
	}
	// -- Parameter data
	parameter = param.parameter;
	parameterType = param.parameterType;
	// -- InBound value
	inBoundVariable = param.inBoundVariable.load(std::memory_order_relaxed);
	// -- Smoothing
	smoothingType = param.smoothingType;
	linearSmoothedValue = juce::LinearSmoothedValue<float>(param.linearSmoothedValue);
	multiplicativeSmoothedValue = juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative>(param.multiplicativeSmoothedValue);

	return *this;
}