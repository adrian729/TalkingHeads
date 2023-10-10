/*
  ==============================================================================

	EQBand.cpp
	Created: 28 Sep 2023 5:10:47pm
	Author:  Brutus729

  ==============================================================================
*/

#include "EQBand.h"

//==============================================================================
// -- CONSTRUCTORS
//==============================================================================
EQBand::EQBand()
{
}

EQBand::~EQBand()
{
	parameterDefinitions = nullptr;
	pluginProcessorParameters = nullptr;
}

//==============================================================================
void EQBand::setupEQBand(
	std::array<ParameterDefinition, ControlID::countParams>& parameterDefinitions,
	std::array<ParameterObject, ControlID::countParams>& pluginProcessorParameters,
	ControlID bypassID,
	ControlID peakFreqID,
	ControlID peakGainID,
	ControlID peakQID
)
{
	this->parameterDefinitions = &parameterDefinitions;
	this->pluginProcessorParameters = &pluginProcessorParameters;
	this->bypassID = bypassID;
	this->peakFreqID = peakFreqID;
	this->peakGainID = peakGainID;
	this->peakQID = peakQID;
}

//==============================================================================
void EQBand::prepare(const juce::dsp::ProcessSpec& spec)
{
	sampleRate = spec.sampleRate;

	peakFreq = (*pluginProcessorParameters)[peakFreqID].getFloatValue();
	peakGain = (*pluginProcessorParameters)[peakGainID].getFloatValue();
	peakQ = (*pluginProcessorParameters)[peakQID].getFloatValue();

	updateCoefficients(
		filter.coefficients,
		makePeakFilter(peakFreq, peakGain, peakQ, sampleRate)
	);

	filter.prepare(spec);
}

void EQBand::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	preProcess();

	if (isBypassed)
	{
		return;
	}

	filter.process(context);
}

void EQBand::reset()
{
	filter.reset();
}

//==============================================================================
float EQBand::getLatency()
{
	return 0.f; // TODO: check if EQ ads latency
}

//==============================================================================
void EQBand::setSampleRate(double sampleRate)
{
	this->sampleRate = sampleRate;
}

//==============================================================================
using Filter = juce::dsp::IIR::Filter<float>;
using CoefficientsPtr = Filter::CoefficientsPtr;
CoefficientsPtr EQBand::makePeakFilter(float peakFreq, float peakGain, float peakQuality, double sampleRate)
{
	return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, peakFreq, peakQuality, juce::Decibels::decibelsToGain(peakGain));
}

void EQBand::updateCoefficients(CoefficientsPtr& old, const CoefficientsPtr& replacements)
{
	*old = *replacements;
}

//==============================================================================
void EQBand::preProcess()
{
	float newBypass = (*pluginProcessorParameters)[bypassID].getCurrentValue();
	bool bypassChanged = !juce::approximatelyEqual(newBypass, bypass);
	bypass = newBypass;

	isBypassed = juce::approximatelyEqual(bypass, 1.0f);
	if (isBypassed)
	{
		return;
	}

	float newPeakFreq = (*pluginProcessorParameters)[peakFreqID].getCurrentValue();
	bool peakFreqChanged = !juce::approximatelyEqual(newPeakFreq, peakFreq);

	float newPeakGain = (*pluginProcessorParameters)[peakGainID].getCurrentValue();
	bool peakGainChanged = !juce::approximatelyEqual(newPeakGain, peakGain);

	float newPeakQ = (*pluginProcessorParameters)[peakQID].getCurrentValue();
	bool peakQChanged = !juce::approximatelyEqual(newPeakQ, peakQ);


	if (bypassChanged || peakFreqChanged || peakGainChanged || peakQChanged)
	{
		peakFreq = newPeakFreq;
		peakGain = newPeakGain;
		peakQ = newPeakQ;

		updateCoefficients(
			filter.coefficients,
			makePeakFilter(peakFreq, peakGain, peakQ * (1.f - bypass), sampleRate)
		);
	}
}
