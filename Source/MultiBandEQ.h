/*
  ==============================================================================

	MultiBandEQ.h
	Created: 1 Sep 2023 1:02:02pm
	Author:  Brutus729

	-- stage 1 -- HPF, LPF, 3 Band EQ

  ==============================================================================
*/

#pragma once

#include <memory>
#include <JuceHeader.h>
#include "parameterTypes.h"
#include "EQBand.h"

//==============================================================================
class MultiBandEQ : public juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	MultiBandEQ();
	~MultiBandEQ();

	//==============================================================================
	// TODO: move setup to constructor as we did with the multiband compressor
	void setupMultiBandEQ(
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
		ControlID bandFilter1BypassID,
		ControlID bandFilter2BypassID,
		ControlID bandFilter3BypassID
	);

	//==============================================================================
	EQBand* getFirstBandFilter();
	EQBand* getSecondBandFilter();
	EQBand* getThirdBandFilter();

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	float getLatency();

	//==============================================================================
	void setSampleRate(double sampleRate);

private:
	// TODO: abstract the class from the processor stage of the plugin so that it can be used separately
	//==============================================================================
	// --- Object parameters management and information
	std::shared_ptr<PluginStateManager> stateManager;

	ControlID highpassBypassID{ ControlID::countParams };
	ControlID highpassFreqID{ ControlID::countParams };
	ControlID highpassSlopeID{ ControlID::countParams };

	ControlID lowpassBypassID{ ControlID::countParams };
	ControlID lowpassFreqID{ ControlID::countParams };
	ControlID lowpassSlopeID{ ControlID::countParams };

	ControlID bandFilter1BypassID{ ControlID::countParams };
	ControlID bandFilter2BypassID{ ControlID::countParams };
	ControlID bandFilter3BypassID{ ControlID::countParams };

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
	bool highpassBypassed{ false };
	float highpassBypass{ 0.f };
	float highpassFreq{ 0.f };
	Slope highpassSlope{ Slope::Slope_12 };
	// -- LPF
	bool lowpassBypassed{ false };
	float lowpassBypass{ 0.f };
	float lowpassFreq{ 0.f };
	Slope lowpassSlope{ Slope::Slope_12 };

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
		EQBand,
		EQBand,
		EQBand,
		PassFilter
	> processorChain;

	//==============================================================================
	// -- Filters
	using Coefficients = juce::dsp::IIR::Coefficients<float>;

	juce::ReferenceCountedArray<Coefficients> makeHighpass(double sampleRate);
	juce::ReferenceCountedArray<Coefficients> makeLowpass(double sampleRate);

	using CoefficientsPtr = Filter::CoefficientsPtr;
	void updateCoefficients(CoefficientsPtr& old, const CoefficientsPtr& replacements);

	template <int Index, typename CoefficientType>
	void update(PassFilter& filter, const CoefficientType& coefficients);

	template <typename CoefficientType>
	void updatePassFilter(PassFilter& passFilter, CoefficientType& coefficients, const Slope& cutSlope);

	//==============================================================================
	void preProcess();
	void postUpdateHighpassFilter();
	void postUpdateLowpassFilter();
	void postUpdateBandFilters();
};
