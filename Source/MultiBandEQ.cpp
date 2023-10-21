/*
  ==============================================================================

	MultiBandEQ.cpp
	Created: 1 Sep 2023 1:02:02pm
	Author:  Brutus729

  ==============================================================================
*/

#include <cmath>
#include "MultiBandEQ.h"

//==============================================================================
// -- CONSTRUCTORS
//==============================================================================
MultiBandEQ::MultiBandEQ(
	std::shared_ptr<PluginStateManager> stateManager,
	// -- HPF
	ControlID highpassBypassID,
	ControlID highpassFreqID,
	ControlID highpassSlopeID,
	// -- LPF
	ControlID lowpassBypassID,
	ControlID lowpassFreqID,
	ControlID lowpassSlopeID,
	// -- Band Filters
	EQBandParamIDs bandFilter1ParamIDs,
	EQBandParamIDs bandFilter2ParamIDs,
	EQBandParamIDs bandFilter3ParamIDs
) :
	stateManager(stateManager),
	// -- HPF
	highpassBypassID(highpassBypassID),
	highpassFreqID(highpassFreqID),
	highpassSlopeID(highpassSlopeID),
	// -- LPF
	lowpassBypassID(lowpassBypassID),
	lowpassFreqID(lowpassFreqID),
	lowpassSlopeID(lowpassSlopeID),
	// -- Band Filters
	bandFilters{
		EQBand(
			stateManager,
			bandFilter1ParamIDs.bypassID,
			bandFilter1ParamIDs.freqID,
			bandFilter1ParamIDs.gainID,
			bandFilter1ParamIDs.qID
		),
		EQBand(
			stateManager,
			bandFilter2ParamIDs.bypassID,
			bandFilter2ParamIDs.freqID,
			bandFilter2ParamIDs.gainID,
			bandFilter2ParamIDs.qID
		),
		EQBand(
			stateManager,
			bandFilter3ParamIDs.bypassID,
			bandFilter3ParamIDs.freqID,
			bandFilter3ParamIDs.gainID,
			bandFilter3ParamIDs.qID
		)
	}
{
}

MultiBandEQ::~MultiBandEQ()
{
}

//==============================================================================
void MultiBandEQ::prepare(const juce::dsp::ProcessSpec& spec)
{
	sampleRate = spec.sampleRate;

	// -- Highpass
	highpassBypass = stateManager->getFloatValue(highpassBypassID);
	highpassBypassed = juce::approximatelyEqual(highpassBypass, 1.0f);
	highpassFreq = stateManager->getFloatValue(highpassFreqID);
	highpassSlope = intToEnum(stateManager->getChoiceIndex(highpassSlopeID), Slope);
	updatePassFilter(highpassFilter, makeHighpass(sampleRate), highpassSlope);
	highpassFilter.prepare(spec);

	// -- Lowpass
	lowpassBypass = stateManager->getFloatValue(lowpassBypassID);
	lowpassBypassed = juce::approximatelyEqual(lowpassBypass, 1.0f);
	lowpassFreq = stateManager->getFloatValue(lowpassFreqID);
	lowpassSlope = intToEnum(stateManager->getChoiceIndex(lowpassSlopeID), Slope);
	updatePassFilter(lowpassFilter, makeLowpass(sampleRate), lowpassSlope);
	lowpassFilter.prepare(spec);

	// -- Bands
	for (auto& bandFilter : bandFilters)
	{
		bandFilter.prepare(spec);
	}
}

void MultiBandEQ::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	preProcess();
	if (!highpassBypassed)
	{
		highpassFilter.process(context);
	}
	if (!lowpassBypassed)
	{
		lowpassFilter.process(context);
	}
	for (auto& bandFilter : bandFilters)
	{
		bandFilter.process(context);
	}
}

void MultiBandEQ::reset()
{
	highpassFilter.reset();
	lowpassFilter.reset();
	for (auto& bandFilter : bandFilters)
	{
		bandFilter.reset();
	}
}

//==============================================================================
void MultiBandEQ::preProcess()
{
	postUpdateHighpassFilter();
	postUpdateLowpassFilter();
}

void MultiBandEQ::postUpdateHighpassFilter()
{
	float newBypass = stateManager->getCurrentValue(highpassBypassID);
	bool bypassChanged = juce::approximatelyEqual(newBypass, highpassBypass);
	highpassBypass = newBypass;
	highpassBypassed = juce::approximatelyEqual(highpassBypass, 1.0f);
	if (highpassBypassed)
	{
		return;
	}

	float newFreq = stateManager->getCurrentValue(highpassFreqID);
	bool freqChanged = juce::approximatelyEqual(newFreq, highpassFreq);

	Slope newSlope = intToEnum(stateManager->getChoiceIndex(highpassSlopeID), Slope);
	bool slopeChanged = newSlope != highpassSlope;


	if (bypassChanged || freqChanged || slopeChanged)
	{
		highpassFreq = newFreq;
		highpassSlope = newSlope;
		updatePassFilter(highpassFilter, makeHighpass(sampleRate), highpassSlope);
	}
}

void MultiBandEQ::postUpdateLowpassFilter()
{
	float newBypass = stateManager->getCurrentValue(lowpassBypassID);
	bool bypassChanged = juce::approximatelyEqual(newBypass, lowpassBypass);
	lowpassBypass = newBypass;
	lowpassBypassed = juce::approximatelyEqual(lowpassBypass, 1.0f);
	if (lowpassBypassed)
	{
		return;
	}

	float newFreq = stateManager->getCurrentValue(lowpassFreqID);
	bool freqChanged = juce::approximatelyEqual(newFreq, lowpassFreq);

	Slope newSlope = intToEnum(stateManager->getChoiceIndex(lowpassSlopeID), Slope);
	bool slopeChanged = newSlope != lowpassSlope;

	if (bypassChanged || freqChanged || slopeChanged)
	{
		lowpassFreq = newFreq;
		lowpassSlope = newSlope;
		updatePassFilter(lowpassFilter, makeLowpass(sampleRate), lowpassSlope);
	}
}

float MultiBandEQ::getLatency()
{
	return 0.f;
}

//==============================================================================
void MultiBandEQ::setSampleRate(double sampleRate)
{
	this->sampleRate = sampleRate;
}

//==============================================================================
// -- Filters
using Coefficients = juce::dsp::IIR::Coefficients<float>;
using Filter = juce::dsp::IIR::Filter<float>;

juce::ReferenceCountedArray<Coefficients> MultiBandEQ::makeHighpass(double sampleRate)
{
	return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
		highpassFreq,
		sampleRate,
		2 * (highpassSlope + 1));
}

juce::ReferenceCountedArray<Coefficients> MultiBandEQ::makeLowpass(double sampleRate)
{
	return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
		lowpassFreq,
		sampleRate,
		2 * (lowpassSlope + 1));
}

using CoefficientsPtr = Filter::CoefficientsPtr;
void MultiBandEQ::updateCoefficients(CoefficientsPtr& old, const CoefficientsPtr& replacements)
{
	*old = *replacements;
}

template <int Index, typename CoefficientType>
void MultiBandEQ::update(PassFilter& filter, const CoefficientType& coefficients)
{
	updateCoefficients(filter.template get<Index>().coefficients, coefficients[Index]);
	filter.template setBypassed<Index>(false);
}

template <typename CoefficientType>
void MultiBandEQ::updatePassFilter(PassFilter& filter, CoefficientType& coefficients, const Slope& slope)
{
	filter.template setBypassed<Slope::Slope_12>(true);
	filter.template setBypassed<Slope::Slope_24>(true);
	filter.template setBypassed<Slope::Slope_36>(true);
	filter.template setBypassed<Slope::Slope_48>(true);

	switch (slope)
	{
	case Slope::Slope_48:
		update<Slope::Slope_48>(filter, coefficients);
		[[fallthrough]];
	case Slope::Slope_36:
		update<Slope::Slope_36>(filter, coefficients);
		[[fallthrough]];
	case Slope::Slope_24:
		update<Slope::Slope_24>(filter, coefficients);
		[[fallthrough]];
	case Slope::Slope_12:
		update<Slope::Slope_12>(filter, coefficients);
	}
}