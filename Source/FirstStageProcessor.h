/*
  ==============================================================================

	FirstStageProcessor.h
	Created: 1 Sep 2023 1:02:02pm
	Author:  Brutus729

	-- stage 1 -- HPF, LPF, 3 Band EQ

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "ParameterDefinition.h"
#include "ParameterObject.h"

//==============================================================================
class FirstStageProcessor : public juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	FirstStageProcessor(
		std::array<ParameterDefinition, ControlID::countParams>(&parameterDefinitions),
		std::array<ParameterObject, ControlID::countParams>(&pluginProcessorParameters)
	);
	~FirstStageProcessor();

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	void preProcess();
	void postProcess();

	void syncInBoundVariables();
	bool postUpdatePluginParameter(ControlID controlID);

	float getLatency();

	//==============================================================================
	void setSampleRate(double sampleRate);

private:
	//==============================================================================
	// --- Object parameters management and information
	const std::set<ControlID> firstStageControlIDs = {
		// -- HPF
		ControlID::highpassFreq,
		ControlID::highpassSlope,
		ControlID::highpassBypass,
		// -- LPF
		ControlID::lowpassFreq,
		ControlID::lowpassSlope,
		ControlID::lowpassBypass,
		// -- Band Filter 1
		ControlID::bandFilter1Bypass,
		ControlID::bandFilter1PeakFreq,
		ControlID::bandFilter1PeakGain,
		ControlID::bandFilter1PeakQ,
		// -- Band Filter 2
		ControlID::bandFilter2Bypass,
		ControlID::bandFilter2PeakFreq,
		ControlID::bandFilter2PeakGain,
		ControlID::bandFilter2PeakQ,
		// -- Band Filter 3
		ControlID::bandFilter3Bypass,
		ControlID::bandFilter3PeakFreq,
		ControlID::bandFilter3PeakGain,
		ControlID::bandFilter3PeakQ
	};

	std::array<ParameterDefinition, ControlID::countParams>(*parameterDefinitions) { nullptr };
	std::array<ParameterObject, ControlID::countParams>(*pluginProcessorParameters) { nullptr };

	//==============================================================================
	// --- Object member variables
	double sampleRate{ 0.f };

	enum Slope
	{
		Slope_12,
		Slope_24,
		Slope_36,
		Slope_48,
		//==============================================================================
		countSlopes // value to keep track of the total number of slopes
	};

	// -- HPF
	float highpassFreq{ 0.f };
	Slope highpassSlope{ Slope::Slope_12 };
	// -- LPF
	float lowpassFreq{ 0.f };
	Slope lowpassSlope{ Slope::Slope_12 };
	// -- Band Filter 1
	float bandFilter1PeakFreq{ 0.f };
	float bandFilter1PeakGain{ 0.f };
	float bandFilter1PeakQ{ 0.f };
	// -- Band Filter 2
	float bandFilter2PeakFreq{ 0.f };
	float bandFilter2PeakGain{ 0.f };
	float bandFilter2PeakQ{ 0.f };
	// -- Band Filter 3
	float bandFilter3PeakFreq{ 0.f };
	float bandFilter3PeakGain{ 0.f };
	float bandFilter3PeakQ{ 0.f };

	//==============================================================================
	// -- Processor Chain
	enum chainIndex
	{
		highpassIndex,
		bandFilter1Index,
		bandFilter2Index,
		bandFilter3Index,
		lowpassIndex
	};

	using Filter = juce::dsp::IIR::Filter<float>;
	using PassFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

	juce::dsp::ProcessorChain<
		PassFilter,
		Filter,
		Filter,
		Filter,
		PassFilter
	> processorChain;

	//==============================================================================
	// -- Filters
	using Coefficients = juce::dsp::IIR::Coefficients<float>;
	juce::ReferenceCountedArray<Coefficients> makeHighpass(double sampleRate);
	juce::ReferenceCountedArray<Coefficients> makeLowpass(double sampleRate);

	using CoefficientsPtr = Filter::CoefficientsPtr;
	CoefficientsPtr makePeakFilter(float peakFreq, float peakGain, float peakQuality, double sampleRate);

	void updateCoefficients(CoefficientsPtr& old, const CoefficientsPtr& replacements);

	template <int Index, typename CoefficientType>
	void update(PassFilter& filter, const CoefficientType& coefficients);

	template <typename CoefficientType>
	void updatePassFilter(PassFilter& passFilter, CoefficientType& coefficients, const Slope& cutSlope);
};
