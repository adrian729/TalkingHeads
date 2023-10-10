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
MultiBandEQ::MultiBandEQ()
{
}

MultiBandEQ::~MultiBandEQ()
{
	parameterDefinitions = nullptr;
	pluginProcessorParameters = nullptr;
}

//==============================================================================
void MultiBandEQ::setupMultiBandEQ(
	std::array<ParameterDefinition, ControlID::countParams>(&parameterDefinitions),
	std::array<ParameterObject, ControlID::countParams>(&pluginProcessorParameters),
	// -- HPF
	ControlID highpassBypassID,
	ControlID highpassFreqID,
	ControlID highpassSlopeID,
	// -- LPF
	ControlID lowpassBypassID,
	ControlID lowpassFreqID,
	ControlID lowpassSlopeID,
	// -- Band Filters
	ControlID bandFilter1BypassID,
	ControlID bandFilter2BypassID,
	ControlID bandFilter3BypassID
)
{
	this->parameterDefinitions = &parameterDefinitions;
	this->pluginProcessorParameters = &pluginProcessorParameters;
	// -- HPF
	this->highpassBypassID = highpassBypassID;
	this->highpassFreqID = highpassFreqID;
	this->highpassSlopeID = highpassSlopeID;
	// -- LPF
	this->lowpassBypassID = lowpassBypassID;
	this->lowpassFreqID = lowpassFreqID;
	this->lowpassSlopeID = lowpassSlopeID;
	// -- Band Filters
	this->bandFilter1BypassID = bandFilter1BypassID;
	this->bandFilter2BypassID = bandFilter2BypassID;
	this->bandFilter3BypassID = bandFilter3BypassID;
}

//==============================================================================
EQBand* MultiBandEQ::getFirstBandFilter()
{
	return &processorChain.template get<bandFilter1Index>();
}

EQBand* MultiBandEQ::getSecondBandFilter()
{
	return &processorChain.template get<bandFilter2Index>();
}

EQBand* MultiBandEQ::getThirdBandFilter()
{
	return &processorChain.template get<bandFilter3Index>();
}

//==============================================================================
void MultiBandEQ::prepare(const juce::dsp::ProcessSpec& spec)
{
	sampleRate = spec.sampleRate;

	// -- Highpass
	highpassFreq = (*pluginProcessorParameters)[ControlID::highpassFreq].getFloatValue();
	highpassSlope = intToEnum((*pluginProcessorParameters)[ControlID::highpassSlope].getChoiceIndex(), Slope);

	auto& highpass = processorChain.get<highpassIndex>();
	updatePassFilter(highpass, makeHighpass(sampleRate), highpassSlope);

	// -- Lowpass
	lowpassFreq = (*pluginProcessorParameters)[ControlID::lowpassFreq].getFloatValue();
	lowpassSlope = intToEnum((*pluginProcessorParameters)[ControlID::lowpassSlope].getChoiceIndex(), Slope);

	auto& lowpass = processorChain.get<lowpassIndex>();
	updatePassFilter(lowpass, makeLowpass(sampleRate), lowpassSlope);

	// -- Chain
	processorChain.prepare(spec);
}

void MultiBandEQ::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	preProcess();

	processorChain.process(context);
}

void MultiBandEQ::reset()
{
	processorChain.reset();
}

//==============================================================================
void MultiBandEQ::preProcess()
{
	postUpdateHighpassFilter();
	postUpdateLowpassFilter();
	postUpdateBandFilters();
}

void MultiBandEQ::postUpdateHighpassFilter()
{
	float newBypass = (*pluginProcessorParameters)[highpassBypassID].getCurrentValue();
	bool bypassChanged = juce::approximatelyEqual(newBypass, highpassBypass);
	highpassBypass = newBypass;
	highpassBypassed = juce::approximatelyEqual(highpassBypass, 1.0f);
	processorChain.template setBypassed<highpassIndex>(highpassBypassed);
	if (highpassBypassed)
	{
		return;
	}

	float newFreq = (*pluginProcessorParameters)[highpassFreqID].getCurrentValue();
	bool freqChanged = juce::approximatelyEqual(newFreq, highpassFreq);

	Slope newSlope = intToEnum((*pluginProcessorParameters)[highpassSlopeID].getChoiceIndex(), Slope);
	bool slopeChanged = newSlope != highpassSlope;


	if (bypassChanged || freqChanged || slopeChanged)
	{
		highpassFreq = newFreq;
		highpassSlope = newSlope;

		auto& highpass = processorChain.template get<highpassIndex>();
		updatePassFilter(highpass, makeHighpass(sampleRate), highpassSlope);
	}
}

void MultiBandEQ::postUpdateLowpassFilter()
{
	float newBypass = (*pluginProcessorParameters)[lowpassBypassID].getCurrentValue();
	bool bypassChanged = juce::approximatelyEqual(newBypass, lowpassBypass);
	lowpassBypass = newBypass;
	lowpassBypassed = juce::approximatelyEqual(lowpassBypass, 1.0f);
	processorChain.template setBypassed<lowpassIndex>(lowpassBypassed);
	if (lowpassBypassed)
	{
		return;
	}

	float newFreq = (*pluginProcessorParameters)[lowpassFreqID].getCurrentValue();
	bool freqChanged = juce::approximatelyEqual(newFreq, lowpassFreq);

	Slope newSlope = intToEnum((*pluginProcessorParameters)[lowpassSlopeID].getChoiceIndex(), Slope);
	bool slopeChanged = newSlope != lowpassSlope;


	if (bypassChanged || freqChanged || slopeChanged)
	{
		lowpassFreq = newFreq;
		lowpassSlope = newSlope;

		auto& lowpass = processorChain.template get<lowpassIndex>();
		updatePassFilter(lowpass, makeLowpass(sampleRate), lowpassSlope);
	}
}

void MultiBandEQ::postUpdateBandFilters()
{
	// -- Band Filter 1
	bool bandFilter1Bypass = juce::approximatelyEqual(
		(*pluginProcessorParameters)[ControlID::bandFilter1Bypass].getCurrentValue(),
		1.f
	);
	processorChain.template setBypassed<bandFilter1Index>(bandFilter1Bypass);

	// -- Band Filter 2
	bool bandFilter2Bypass = juce::approximatelyEqual(
		(*pluginProcessorParameters)[ControlID::bandFilter2Bypass].getCurrentValue(),
		1.f
	);
	processorChain.template setBypassed<bandFilter2Index>(bandFilter2Bypass);


	// -- Band Filter 3
	bool bandFilter3Bypass = juce::approximatelyEqual(
		(*pluginProcessorParameters)[ControlID::bandFilter3Bypass].getCurrentValue(),
		1.f
	);
	processorChain.template setBypassed<bandFilter3Index>(bandFilter3Bypass);
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