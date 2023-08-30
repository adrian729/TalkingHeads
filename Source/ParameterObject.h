/*
  ==============================================================================

	ParametersObject.h
	Parameter information for the plugin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"

struct SmoothingObject
{
	//==============================================================================
	// -- Constructors & Destructors
	//==============================================================================
	SmoothingObject() : smoothingType(SmoothingType::NoSmoothing),
		range(juce::NormalisableRange<float>())
	{
	}

	SmoothingObject(SmoothingType smoothingType, const juce::NormalisableRange<float>& range) :
		smoothingType(smoothingType),
		range(juce::NormalisableRange(range))
	{
	}

	~SmoothingObject()
	{
	}

	//==============================================================================
	// -- Members
	//==============================================================================
	SmoothingType smoothingType;
	juce::NormalisableRange<float> range;
	// -- Linear
	juce::LinearSmoothedValue<float> linearSmoothedValue;
	// -- Multiplicative
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> multiplicativeSmoothedValue;

	//==============================================================================
	// -- Methods
	//==============================================================================
	bool reset(double sampleRate, double rampLengthInSeconds)
	{
		if (smoothingType == SmoothingType::NoSmoothing)
		{
			return false;
		}

		switch (smoothingType)
		{
		case SmoothingType::Linear:
			linearSmoothedValue.reset(sampleRate, 0.5f);
			break;
		case SmoothingType::Multiplicative:
			multiplicativeSmoothedValue.reset(sampleRate, rampLengthInSeconds);
			break;
		default:
			return false;
		}

		return true;
	}

	bool setCurrentAndTargetValue(float newValue)
	{
		if (smoothingType == SmoothingType::NoSmoothing)
		{
			return false;
		}
		// Need to normalize the inBound variable to the range [0, 1] first
		float normalizedNewValue = range.convertTo0to1(newValue);

		switch (smoothingType)
		{
		case SmoothingType::Linear:
			linearSmoothedValue.setCurrentAndTargetValue(normalizedNewValue);
			break;
		case SmoothingType::Multiplicative:
			multiplicativeSmoothedValue.setCurrentAndTargetValue(safeMultiplicativeValue(normalizedNewValue));
			break;
		default:
			return false;
		}

		return true;
	}

	bool setTargetValue(float newValue)
	{
		if (smoothingType == SmoothingType::NoSmoothing)
		{
			return false;
		}
		// Need to normalize the inBound variable to the range [0, 1] first
		float normalizedNewValue = range.convertTo0to1(newValue);

		switch (smoothingType)
		{
		case SmoothingType::Linear:
			linearSmoothedValue.setTargetValue(normalizedNewValue);
			break;
		case SmoothingType::Multiplicative:
			multiplicativeSmoothedValue.setTargetValue(safeMultiplicativeValue(normalizedNewValue));
			break;
		}

		return true;
	}

	float getCurrentValue()
	{
		if (smoothingType == SmoothingType::NoSmoothing)
		{
			return 0.f;
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
		return range.convertFrom0to1(normalizedValue);
	}

	float getNextValue()
	{
		if (smoothingType == SmoothingType::NoSmoothing)
		{
			return 0.f;
		}

		//setTargetValue(0.7f);
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
		return range.convertFrom0to1(normalizedValue);
	}

	//==============================================================================
	float safeMultiplicativeValue(float value, float smallestValue = 0.0001f)
	{
		return juce::approximatelyEqual(value, 0.f) ? smallestValue : value;
	}

	//==============================================================================
	// TODO: make sure this is correct for the smoothed values
	SmoothingObject& operator=(const SmoothingObject& newObject)
	{
		if (this == &newObject)
		{
			return *this;
		}

		smoothingType = newObject.smoothingType;
		range = juce::NormalisableRange(newObject.range);
		linearSmoothedValue.setCurrentAndTargetValue(newObject.linearSmoothedValue.getCurrentValue());
		multiplicativeSmoothedValue.setCurrentAndTargetValue(newObject.multiplicativeSmoothedValue.getCurrentValue());

		return *this;
	}
};

class ParameterObject
{
public:
	//==============================================================================
	ParameterObject();
	ParameterObject(juce::RangedAudioParameter* parameter, ParameterType parameterType = ParameterType::Float);
	ParameterObject(juce::RangedAudioParameter* parameter, SmoothingType smoothingType);
	~ParameterObject();

	//==============================================================================
	// -- Parameter getters and setters
	juce::RangedAudioParameter* getParameter();
	ParameterType getParameterType();

	juce::AudioParameterBool* getParameterBool();
	juce::AudioParameterChoice* getParameterChoice();
	juce::AudioParameterFloat* getParameterFloat();
	juce::AudioParameterInt* getParameterInt();

	//==============================================================================
	// -- InBound variable getters and setters
	float getInBoundVariable();

	bool getBoolValue();
	int getChoiceIndex();
	template <typename ChoiceType>
	ChoiceType getChoiceValue();
	float getFloatValue();
	int getIntValue();

	//==============================================================================
	// -- Smoothing
	float getSmoothedCurrentValue();
	float getSmoothedValue();

	void initSmoothing(double sampleRate, double rampLengthInSeconds = 0.005f);

	//==============================================================================
	void updateInBoundVariable();

	//==============================================================================
	ParameterObject& operator=(const ParameterObject& params); // ?: need this override for collections to work

private:
	//==============================================================================
	// -- Parameter data
	juce::RangedAudioParameter* parameter{ nullptr };
	ParameterType parameterType{ ParameterType::Float };

	// -- InBound value
	std::atomic<float> inBoundVariable;
	SmoothingObject smoothedVariable;
};
