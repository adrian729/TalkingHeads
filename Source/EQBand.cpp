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
EQBand::EQBand(
	std::shared_ptr<PluginStateManager> stateManager,
	ControlID bypassID,
	ControlID peakFreqID,
	ControlID peakGainID,
	ControlID peakQID
) :
	stateManager(stateManager),
	bypassID(bypassID),
	peakFreqID(peakFreqID),
	peakGainID(peakGainID),
	peakQID(peakQID)
{
}

EQBand::~EQBand()
{
}

//==============================================================================
void EQBand::prepare(const juce::dsp::ProcessSpec& spec)
{
	sampleRate = spec.sampleRate;

	peakFreq = stateManager->getFloatValue(peakFreqID);
	peakGain = stateManager->getFloatValue(peakGainID);
	peakQ = stateManager->getFloatValue(peakQID);

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
	float newBypass = stateManager->getCurrentValue(bypassID);
	bool bypassChanged = !juce::approximatelyEqual(newBypass, bypass);
	bypass = newBypass;

	isBypassed = juce::approximatelyEqual(bypass, 1.0f);
	if (isBypassed)
	{
		return;
	}

	float newPeakFreq = stateManager->getCurrentValue(peakFreqID);
	bool peakFreqChanged = !juce::approximatelyEqual(newPeakFreq, peakFreq);

	float newPeakGain = stateManager->getCurrentValue(peakGainID);
	bool peakGainChanged = !juce::approximatelyEqual(newPeakGain, peakGain);

	float newPeakQ = stateManager->getCurrentValue(peakQID);
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
