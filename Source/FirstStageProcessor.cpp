/*
  ==============================================================================

	FirstStageProcessor.cpp
	Created: 1 Sep 2023 1:02:02pm
	Author:  Brutus729

  ==============================================================================
*/

#include "FirstStageProcessor.h"

//==============================================================================
// -- CONSTRUCTORS
//==============================================================================
FirstStageProcessor::FirstStageProcessor(
	std::array<ParameterDefinition, ControlID::countParams>(&parameterDefinitions),
	std::array<ParameterObject, ControlID::countParams>(&pluginProcessorParameters)
) :
	parameterDefinitions(&parameterDefinitions),
	pluginProcessorParameters(&pluginProcessorParameters)
{
}

FirstStageProcessor::~FirstStageProcessor()
{
	parameterDefinitions = nullptr;
	pluginProcessorParameters = nullptr;
}

//==============================================================================
void FirstStageProcessor::prepare(const juce::dsp::ProcessSpec& spec)
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

	// -- Band Filter 1
	bandFilter1PeakFreq = (*pluginProcessorParameters)[ControlID::bandFilter1PeakFreq].getFloatValue();
	bandFilter1PeakGain = (*pluginProcessorParameters)[ControlID::bandFilter1PeakGain].getFloatValue();
	bandFilter1PeakQ = (*pluginProcessorParameters)[ControlID::bandFilter1PeakQ].getFloatValue();

	auto& bandFilter1 = processorChain.get<bandFilter1Index>();
	updateCoefficients(
		bandFilter1.coefficients,
		makePeakFilter(bandFilter1PeakFreq, bandFilter1PeakGain, bandFilter1PeakQ, sampleRate)
	);

	// -- Band Filter 2
	bandFilter2PeakFreq = (*pluginProcessorParameters)[ControlID::bandFilter2PeakFreq].getFloatValue();
	bandFilter2PeakGain = (*pluginProcessorParameters)[ControlID::bandFilter2PeakGain].getFloatValue();
	bandFilter2PeakQ = (*pluginProcessorParameters)[ControlID::bandFilter2PeakQ].getFloatValue();

	auto& bandFilter2 = processorChain.get<bandFilter2Index>();
	updateCoefficients(
		bandFilter2.coefficients,
		makePeakFilter(bandFilter2PeakFreq, bandFilter2PeakGain, bandFilter2PeakQ, sampleRate)
	);

	// -- Band Filter 3
	bandFilter3PeakFreq = (*pluginProcessorParameters)[ControlID::bandFilter3PeakFreq].getFloatValue();
	bandFilter3PeakGain = (*pluginProcessorParameters)[ControlID::bandFilter3PeakGain].getFloatValue();
	bandFilter3PeakQ = (*pluginProcessorParameters)[ControlID::bandFilter3PeakQ].getFloatValue();

	auto& bandFilter3 = processorChain.get<bandFilter3Index>();
	updateCoefficients(
		bandFilter3.coefficients,
		makePeakFilter(bandFilter3PeakFreq, bandFilter3PeakGain, bandFilter3PeakQ, sampleRate)
	);

	// -- Chain
	processorChain.prepare(spec);

	// -- Setup smoothing
	// -- HPF/LPF
	(*pluginProcessorParameters)[ControlID::highpassFreq].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::lowpassFreq].initSmoothing(sampleRate);
	// -- Band 1
	(*pluginProcessorParameters)[ControlID::bandFilter1PeakFreq].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::bandFilter1PeakGain].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::bandFilter1PeakQ].initSmoothing(sampleRate);
	// -- Band 2
	(*pluginProcessorParameters)[ControlID::bandFilter2PeakFreq].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::bandFilter2PeakGain].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::bandFilter2PeakQ].initSmoothing(sampleRate);
	// -- Band 3
	(*pluginProcessorParameters)[ControlID::bandFilter3PeakFreq].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::bandFilter3PeakGain].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::bandFilter3PeakQ].initSmoothing(sampleRate);

}

void FirstStageProcessor::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	preProcess();

	processorChain.process(context);

	postProcess();
}

void FirstStageProcessor::reset()
{
	processorChain.reset();
}

//==============================================================================
void FirstStageProcessor::preProcess()
{
	syncInBoundVariables();
}

void FirstStageProcessor::postProcess()
{
}

void FirstStageProcessor::syncInBoundVariables()
{
	for (auto& id : firstStageControlIDs)
	{
		(*pluginProcessorParameters)[id].updateInBoundVariable();
		postUpdatePluginParameter(id);
	}

	// -- HPF
	bool highpassBypass = (*pluginProcessorParameters)[ControlID::highpassBypass].getBoolValue();
	processorChain.template setBypassed<highpassIndex>(highpassBypass);
	if (!highpassBypass)
	{
		auto& highpass = processorChain.get<highpassIndex>();
		updatePassFilter(highpass, makeHighpass(sampleRate), highpassSlope);
	}
	// -- LPF
	bool lowpassBypass = (*pluginProcessorParameters)[ControlID::lowpassBypass].getBoolValue();
	processorChain.template setBypassed<lowpassIndex>(lowpassBypass);
	if (!lowpassBypass)
	{
		auto& lowpass = processorChain.get<lowpassIndex>();
		updatePassFilter(lowpass, makeLowpass(sampleRate), lowpassSlope);
	}

	// -- Band Filter 1
	bool bandFilter1Bypass = (*pluginProcessorParameters)[ControlID::bandFilter1Bypass].getBoolValue();
	processorChain.template setBypassed<bandFilter1Index>(bandFilter1Bypass);
	if (!bandFilter1Bypass)
	{
		auto& bandFilter1 = processorChain.get<bandFilter1Index>();
		updateCoefficients(
			bandFilter1.coefficients,
			makePeakFilter(bandFilter1PeakFreq, bandFilter1PeakGain, bandFilter1PeakQ, sampleRate)
		);
	}

	// -- Band Filter 2
	bool bandFilter2Bypass = (*pluginProcessorParameters)[ControlID::bandFilter2Bypass].getBoolValue();
	processorChain.template setBypassed<bandFilter2Index>(bandFilter2Bypass);
	if (!bandFilter2Bypass)
	{
		auto& bandFilter2 = processorChain.get<bandFilter2Index>();
		updateCoefficients(
			bandFilter2.coefficients,
			makePeakFilter(bandFilter2PeakFreq, bandFilter2PeakGain, bandFilter2PeakQ, sampleRate)
		);
	}

	// -- Band Filter 3
	bool bandFilter3Bypass = (*pluginProcessorParameters)[ControlID::bandFilter3Bypass].getBoolValue();
	processorChain.template setBypassed<bandFilter3Index>(bandFilter3Bypass);
	if (!bandFilter3Bypass)
	{
		auto& bandFilter3 = processorChain.get<bandFilter3Index>();
		updateCoefficients(
			bandFilter3.coefficients,
			makePeakFilter(bandFilter3PeakFreq, bandFilter3PeakGain, bandFilter3PeakQ, sampleRate)
		);
	}
}

bool FirstStageProcessor::postUpdatePluginParameter(ControlID controlID)
{
	// Cooking and transfer raw parameters to member variables
	switch (controlID)
	{
		// -- HPF
	case ControlID::highpassFreq:
	{
		highpassFreq = (*pluginProcessorParameters)[ControlID::highpassFreq].getNextValue();
		break;
	}
	case ControlID::highpassSlope:
	{
		int slopeIndex = (*pluginProcessorParameters)[ControlID::highpassSlope].getChoiceIndex();
		highpassSlope = intToEnum(slopeIndex, Slope);
		break;
	}

	// -- LPF
	case ControlID::lowpassFreq:
	{
		lowpassFreq = (*pluginProcessorParameters)[ControlID::lowpassFreq].getNextValue();
		break;
	}
	case ControlID::lowpassSlope:
	{
		int slopeIndex = (*pluginProcessorParameters)[ControlID::lowpassSlope].getChoiceIndex();
		lowpassSlope = intToEnum(slopeIndex, Slope);
		break;
	}

	// -- Band Filter 1
	case ControlID::bandFilter1PeakFreq:
	{
		bandFilter1PeakFreq = (*pluginProcessorParameters)[ControlID::bandFilter1PeakFreq].getNextValue();
		break;
	}
	case ControlID::bandFilter1PeakGain:
	{
		bandFilter1PeakGain = (*pluginProcessorParameters)[ControlID::bandFilter1PeakGain].getNextValue();
		break;
	}
	case ControlID::bandFilter1PeakQ:
	{
		bandFilter1PeakQ = (*pluginProcessorParameters)[ControlID::bandFilter1PeakQ].getNextValue();
		break;
	}
	// -- Band Filter 2
	case ControlID::bandFilter2PeakFreq:
	{
		bandFilter2PeakFreq = (*pluginProcessorParameters)[ControlID::bandFilter2PeakFreq].getNextValue();
		break;
	}
	case ControlID::bandFilter2PeakGain:
	{
		bandFilter2PeakGain = (*pluginProcessorParameters)[ControlID::bandFilter2PeakGain].getNextValue();
		break;
	}
	case ControlID::bandFilter2PeakQ:
	{
		bandFilter2PeakQ = (*pluginProcessorParameters)[ControlID::bandFilter2PeakQ].getNextValue();
		break;
	}
	// -- Band Filter 3
	case ControlID::bandFilter3PeakFreq:
	{
		bandFilter3PeakFreq = (*pluginProcessorParameters)[ControlID::bandFilter3PeakFreq].getNextValue();
		break;
	}
	case ControlID::bandFilter3PeakGain:
	{
		bandFilter3PeakGain = (*pluginProcessorParameters)[ControlID::bandFilter3PeakGain].getNextValue();
		break;
	}
	case ControlID::bandFilter3PeakQ:
	{
		bandFilter3PeakQ = (*pluginProcessorParameters)[ControlID::bandFilter3PeakQ].getNextValue();
		break;
	}

	default:
		return false;
	}

	return true;
}

float FirstStageProcessor::getLatency()
{
	return 0.f;
}

//==============================================================================
void FirstStageProcessor::setSampleRate(double sampleRate)
{
	this->sampleRate = sampleRate;
}

//==============================================================================
// -- Filters
using Coefficients = juce::dsp::IIR::Coefficients<float>;
using Filter = juce::dsp::IIR::Filter<float>;

juce::ReferenceCountedArray<Coefficients> FirstStageProcessor::makeHighpass(double sampleRate)
{
	return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
		highpassFreq,
		sampleRate,
		2 * (highpassSlope + 1));
}

juce::ReferenceCountedArray<Coefficients> FirstStageProcessor::makeLowpass(double sampleRate)
{
	return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
		lowpassFreq,
		sampleRate,
		2 * (lowpassSlope + 1));
}

using CoefficientsPtr = Filter::CoefficientsPtr;

CoefficientsPtr FirstStageProcessor::makePeakFilter(float peakFreq, float peakGain, float peakQuality, double sampleRate)
{
	return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, peakFreq, peakQuality, juce::Decibels::decibelsToGain(peakGain));
}

void FirstStageProcessor::updateCoefficients(CoefficientsPtr& old, const CoefficientsPtr& replacements)
{
	*old = *replacements;
}

template <int Index, typename CoefficientType>
void FirstStageProcessor::update(PassFilter& filter, const CoefficientType& coefficients)
{
	updateCoefficients(filter.template get<Index>().coefficients, coefficients[Index]);
	filter.template setBypassed<Index>(false);
}

template <typename CoefficientType>
void FirstStageProcessor::updatePassFilter(PassFilter& filter, CoefficientType& coefficients, const Slope& slope)
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