/*
  ==============================================================================

	ParametersObject.h
	Parameter information for the plugin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"

class ParameterObject
{
public:
	//==============================================================================
	// -- Constructors & Destructors
	ParameterObject();
	ParameterObject(juce::RangedAudioParameter* parameter, ParameterType parameterType = ParameterType::Float, SmoothingType smoothingType = SmoothingType::NoSmoothing);
	~ParameterObject();

	//==============================================================================
	// -- Parameter getters and setters
	ParameterType getParameterType();
	juce::NormalisableRange<float> getParameterRange();

	juce::RangedAudioParameter* getParameter();
	template <typename T>
	T getParameter();
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
	void initSmoothing(double sampleRate, double rampLengthInSeconds = 0.005f);

	float getCurrentValue();
	float getNextValue();

	void setCurrentAndTargetValue(float newValue);
	void setTargetValue(float newValue);

	void reset(double sampleRate, double rampLengthInSeconds);

	//==============================================================================
	void updateInBoundVariable();

	//==============================================================================
	ParameterObject& operator=(const ParameterObject& params); // ?: need this override for collections to work

private:
	//==============================================================================
	// -- Parameter data
	juce::RangedAudioParameter* parameter;
	ParameterType parameterType;
	//==============================================================================
	// -- InBound value
	std::atomic<float> inBoundVariable;
	//==============================================================================
	// -- Smoothing 
	SmoothingType smoothingType;
	// -- Linear
	juce::LinearSmoothedValue<float> linearSmoothedValue;
	// -- Multiplicative
	juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> multiplicativeSmoothedValue;

	//==============================================================================
	float safeMultiplicativeValue(float value, float smallestValue = 0.0001f);

};
