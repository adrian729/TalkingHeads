/*
  ==============================================================================

	EQBand.h
	Created: 28 Sep 2023 5:10:47pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "ParameterDefinition.h"
#include "ParameterObject.h"

//==============================================================================
class EQBand : public  juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	EQBand();
	~EQBand();

	//==============================================================================
	void setupEQBand(
		std::array<ParameterDefinition, ControlID::countParams>& parameterDefinitions,
		std::array<ParameterObject, ControlID::countParams>& pluginProcessorParameters,
		ControlID bypassID,
		ControlID peakFreqID,
		ControlID peakGainID,
		ControlID peakQID
	);

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	float getLatency();

	//==============================================================================
	void setSampleRate(double sampleRate);

private:
	//==============================================================================
	// --- Object parameters management and information
	ControlID bypassID{ ControlID::countParams };
	ControlID peakFreqID{ ControlID::countParams };
	ControlID peakGainID{ ControlID::countParams };
	ControlID peakQID{ ControlID::countParams };

	std::array<ParameterDefinition, ControlID::countParams>(*parameterDefinitions) { nullptr };
	std::array<ParameterObject, ControlID::countParams>(*pluginProcessorParameters) { nullptr };

	//==============================================================================
	// --- Object member variables
	double sampleRate{ 0.f };

	bool isBypassed{ false };
	float bypass{ 0.f };
	float peakFreq{ 0.f };
	float peakGain{ 0.f };
	float peakQ{ 0.f };

	using Filter = juce::dsp::IIR::Filter<float>;
	Filter filter;

	//==============================================================================
	using CoefficientsPtr = Filter::CoefficientsPtr;
	CoefficientsPtr makePeakFilter(float peakFreq, float peakGain, float peakQuality, double sampleRate);

	void updateCoefficients(CoefficientsPtr& old, const CoefficientsPtr& replacements);

	//==============================================================================
	void preProcess();
};
