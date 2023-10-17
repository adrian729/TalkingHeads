/*
  ==============================================================================

	Class to store the raw parameter value

  ==============================================================================
*/

#include "ParameterObject.h"

//==============================================================================
ParameterObject::ParameterObject() :
	parameter(nullptr),
	parameterType(ParameterType::Float)
{
}

ParameterObject::ParameterObject(juce::RangedAudioParameter* parameter, ParameterType parameterType) :
	parameter(parameter),
	parameterType(parameterType)
{
	// -- Update the inBound variables to initialize them
	updateInBoundVariable();
}

ParameterObject::~ParameterObject()
{
	parameter = nullptr;
}

ParameterObject::ParameterObject(const ParameterObject& other) : // copy constructor
	parameter(other.parameter),
	parameterType(other.parameterType),
	inBoundVariable(other.inBoundVariable.load(std::memory_order_relaxed))
{
}

ParameterObject::ParameterObject(ParameterObject&& other) noexcept : // move constructor
	parameter(std::exchange(other.parameter, nullptr)),
	parameterType(other.parameterType),
	inBoundVariable(other.inBoundVariable.load(std::memory_order_relaxed))
{
}

//==============================================================================
ParameterObject& ParameterObject::operator=(const ParameterObject& other) // copy assignment
{
	if (this == &other)
	{
		return *this;
	}
	// -- Parameter data
	parameter = other.parameter;
	parameterType = other.parameterType;
	// -- InBound value
	inBoundVariable = other.inBoundVariable.load(std::memory_order_relaxed);

	return *this;
}

ParameterObject& ParameterObject::operator=(ParameterObject&& other) noexcept // move assignment
{
	if (this == &other)
	{
		return *this;
	}
	// -- Parameter data
	std::swap(parameterType, other.parameterType);
	std::swap(parameter, other.parameter);
	// -- InBound value
	float tempInBoundVariable = inBoundVariable.load(std::memory_order_relaxed);
	inBoundVariable = other.inBoundVariable.load(std::memory_order_relaxed);
	other.inBoundVariable = tempInBoundVariable;

	return *this;
}

//==============================================================================
// -- Parameter
ParameterType ParameterObject::getParameterType()
{
	return parameterType;
}

juce::NormalisableRange<float> ParameterObject::getParameterRange()
{
	return parameter->getNormalisableRange();
}

juce::RangedAudioParameter* ParameterObject::getParameter()
{
	return parameter;
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

template <typename T>
T ParameterObject::getParameter()
{
	T t_parameter = dynamic_cast<T>(parameter);
	jassert(t_parameter != nullptr);
	return t_parameter;
}


//==============================================================================
// -- InBound variable
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
float ParameterObject::updateInBoundVariable()
{
	// -- Update the inBound variable
	switch (parameterType)
	{
	case ParameterType::Bool:
		inBoundVariable = static_cast<float>(getParameterBool()->get());
		break;
	case ParameterType::Choice:
		inBoundVariable = static_cast<float>(getParameterChoice()->getIndex());
		break;
	case ParameterType::Int:
		inBoundVariable = static_cast<float>(getParameterInt()->get());
		break;
	case ParameterType::Float:
		inBoundVariable = getParameterFloat()->get();
	}

	return getInBoundVariable();
}
