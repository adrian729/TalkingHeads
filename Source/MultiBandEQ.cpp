/*
  ==============================================================================

	MultiBandEQ.cpp
	Created: 1 Sep 2023 1:02:02pm
	Author:  Brutus729

  ==============================================================================
*/

#include "MultiBandEQ.h"

//==============================================================================
// -- CONSTRUCTORS
//==============================================================================
MultiBandEQ::MultiBandEQ(
	std::array<ParameterDefinition, ControlID::countParams>(&parameterDefinitions),
	std::array<ParameterObject, ControlID::countParams>(&pluginProcessorParameters)) : parameterDefinitions(&parameterDefinitions),
																					   pluginProcessorParameters(&pluginProcessorParameters)
{
}

MultiBandEQ::~MultiBandEQ()
{
	parameterDefinitions = nullptr;
	pluginProcessorParameters = nullptr;
}

//==============================================================================
void MultiBandEQ::prepare(const juce::dsp::ProcessSpec &spec)
{
	sampleRate = spec.sampleRate;

	// -- Highpass
	highpassFreq = (*pluginProcessorParameters)[ControlID::highpassFreq].getFloatValue();
	highpassSlope = intToEnum((*pluginProcessorParameters)[ControlID::highpassSlope].getChoiceIndex(), Slope);

	auto &highpass = processorChain.get<highpassIndex>();
	updatePassFilter(highpass, makeHighpass(sampleRate), highpassSlope);

	// -- Lowpass
	lowpassFreq = (*pluginProcessorParameters)[ControlID::lowpassFreq].getFloatValue();
	lowpassSlope = intToEnum((*pluginProcessorParameters)[ControlID::lowpassSlope].getChoiceIndex(), Slope);

	auto &lowpass = processorChain.get<lowpassIndex>();
	updatePassFilter(lowpass, makeLowpass(sampleRate), lowpassSlope);

	// -- Band Filter 1
	bandFilter1PeakFreq = (*pluginProcessorParameters)[ControlID::bandFilter1PeakFreq].getFloatValue();
	bandFilter1PeakGain = (*pluginProcessorParameters)[ControlID::bandFilter1PeakGain].getFloatValue();
	bandFilter1PeakQ = (*pluginProcessorParameters)[ControlID::bandFilter1PeakQ].getFloatValue();

	auto &bandFilter1 = processorChain.get<bandFilter1Index>();
	updateCoefficients(
		bandFilter1.coefficients,
		makePeakFilter(bandFilter1PeakFreq, bandFilter1PeakGain, bandFilter1PeakQ, sampleRate));

	// -- Band Filter 2
	bandFilter2PeakFreq = (*pluginProcessorParameters)[ControlID::bandFilter2PeakFreq].getFloatValue();
	bandFilter2PeakGain = (*pluginProcessorParameters)[ControlID::bandFilter2PeakGain].getFloatValue();
	bandFilter2PeakQ = (*pluginProcessorParameters)[ControlID::bandFilter2PeakQ].getFloatValue();

	auto &bandFilter2 = processorChain.get<bandFilter2Index>();
	updateCoefficients(
		bandFilter2.coefficients,
		makePeakFilter(bandFilter2PeakFreq, bandFilter2PeakGain, bandFilter2PeakQ, sampleRate));

	// -- Band Filter 3
	bandFilter3PeakFreq = (*pluginProcessorParameters)[ControlID::bandFilter3PeakFreq].getFloatValue();
	bandFilter3PeakGain = (*pluginProcessorParameters)[ControlID::bandFilter3PeakGain].getFloatValue();
	bandFilter3PeakQ = (*pluginProcessorParameters)[ControlID::bandFilter3PeakQ].getFloatValue();

	auto &bandFilter3 = processorChain.get<bandFilter3Index>();
	updateCoefficients(
		bandFilter3.coefficients,
		makePeakFilter(bandFilter3PeakFreq, bandFilter3PeakGain, bandFilter3PeakQ, sampleRate));

	// -- Chain
	processorChain.prepare(spec);
}

void MultiBandEQ::process(const juce::dsp::ProcessContextReplacing<float> &context)
{
	preProcess();

	processorChain.process(context);

	postProcess();
}

void MultiBandEQ::reset()
{
	processorChain.reset();
}

//==============================================================================
void MultiBandEQ::preProcess()
{
	syncInBoundVariables();
}

void MultiBandEQ::postProcess()
{
}

void MultiBandEQ::syncInBoundVariables()
{
	for (auto &id : firstStageControlIDs)
	{
		postUpdatePluginParameter(id);
	}

	// -- HPF
	bool highpassBypass = (*pluginProcessorParameters)[ControlID::highpassBypass].getBoolValue();
	processorChain.template setBypassed<highpassIndex>(highpassBypass);
	if (!highpassBypass)
	{
		auto &highpass = processorChain.get<highpassIndex>();
		updatePassFilter(highpass, makeHighpass(sampleRate), highpassSlope);
	}
	// -- LPF
	bool lowpassBypass = (*pluginProcessorParameters)[ControlID::lowpassBypass].getBoolValue();
	processorChain.template setBypassed<lowpassIndex>(lowpassBypass);
	if (!lowpassBypass)
	{
		auto &lowpass = processorChain.get<lowpassIndex>();
		updatePassFilter(lowpass, makeLowpass(sampleRate), lowpassSlope);
	}

	// -- Band Filter 1
	bool bandFilter1Bypass = (*pluginProcessorParameters)[ControlID::bandFilter1Bypass].getBoolValue();
	processorChain.template setBypassed<bandFilter1Index>(bandFilter1Bypass);
	if (!bandFilter1Bypass)
	{
		auto &bandFilter1 = processorChain.get<bandFilter1Index>();
		updateCoefficients(
			bandFilter1.coefficients,
			makePeakFilter(bandFilter1PeakFreq, bandFilter1PeakGain, bandFilter1PeakQ, sampleRate));
	}

	// -- Band Filter 2
	bool bandFilter2Bypass = (*pluginProcessorParameters)[ControlID::bandFilter2Bypass].getBoolValue();
	processorChain.template setBypassed<bandFilter2Index>(bandFilter2Bypass);
	if (!bandFilter2Bypass)
	{
		auto &bandFilter2 = processorChain.get<bandFilter2Index>();
		updateCoefficients(
			bandFilter2.coefficients,
			makePeakFilter(bandFilter2PeakFreq, bandFilter2PeakGain, bandFilter2PeakQ, sampleRate));
	}

	// -- Band Filter 3
	bool bandFilter3Bypass = (*pluginProcessorParameters)[ControlID::bandFilter3Bypass].getBoolValue();
	processorChain.template setBypassed<bandFilter3Index>(bandFilter3Bypass);
	if (!bandFilter3Bypass)
	{
		auto &bandFilter3 = processorChain.get<bandFilter3Index>();
		updateCoefficients(
			bandFilter3.coefficients,
			makePeakFilter(bandFilter3PeakFreq, bandFilter3PeakGain, bandFilter3PeakQ, sampleRate));
	}
}

bool MultiBandEQ::postUpdatePluginParameter(ControlID controlID)
{
	// Cooking and transfer raw parameters to member variables
	switch (controlID)
	{
		// -- HPF
	case ControlID::highpassFreq:
	{
		highpassFreq = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	case ControlID::highpassSlope:
	{
		int slopeIndex = (*pluginProcessorParameters)[controlID].getChoiceIndex();
		highpassSlope = intToEnum(slopeIndex, Slope);
		break;
	}

	// -- LPF
	case ControlID::lowpassFreq:
	{
		lowpassFreq = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	case ControlID::lowpassSlope:
	{
		int slopeIndex = (*pluginProcessorParameters)[controlID].getChoiceIndex();
		lowpassSlope = intToEnum(slopeIndex, Slope);
		break;
	}

	// -- Band Filter 1
	case ControlID::bandFilter1PeakFreq:
	{
		bandFilter1PeakFreq = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	case ControlID::bandFilter1PeakGain:
	{
		bandFilter1PeakGain = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	case ControlID::bandFilter1PeakQ:
	{
		bandFilter1PeakQ = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	// -- Band Filter 2
	case ControlID::bandFilter2PeakFreq:
	{
		bandFilter2PeakFreq = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	case ControlID::bandFilter2PeakGain:
	{
		bandFilter2PeakGain = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	case ControlID::bandFilter2PeakQ:
	{
		bandFilter2PeakQ = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	// -- Band Filter 3
	case ControlID::bandFilter3PeakFreq:
	{
		bandFilter3PeakFreq = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	case ControlID::bandFilter3PeakGain:
	{
		bandFilter3PeakGain = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}
	case ControlID::bandFilter3PeakQ:
	{
		bandFilter3PeakQ = (*pluginProcessorParameters)[controlID].getCurrentValue();
		break;
	}

	default:
		return false;
	}

	return true;
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

CoefficientsPtr MultiBandEQ::makePeakFilter(float peakFreq, float peakGain, float peakQuality, double sampleRate)
{
	return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, peakFreq, peakQuality, juce::Decibels::decibelsToGain(peakGain));
}

void MultiBandEQ::updateCoefficients(CoefficientsPtr &old, const CoefficientsPtr &replacements)
{
	*old = *replacements;
}

template <int Index, typename CoefficientType>
void MultiBandEQ::update(PassFilter &filter, const CoefficientType &coefficients)
{
	updateCoefficients(filter.template get<Index>().coefficients, coefficients[Index]);
	filter.template setBypassed<Index>(false);
}

template <typename CoefficientType>
void MultiBandEQ::updatePassFilter(PassFilter &filter, CoefficientType &coefficients, const Slope &slope)
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