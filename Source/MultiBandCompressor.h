/*
  ==============================================================================

	MultiBandCompressor.h
	Created: 22 Sep 2023 5:01:45pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "ParameterDefinition.h"
#include "ParameterObject.h"
#include "CompressorBand.h"

//==============================================================================
class MultiBandCompressor : public juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	MultiBandCompressor(
		std::array<ParameterDefinition, ControlID::countParams>(&parameterDefinitions),
		std::array<ParameterObject, ControlID::countParams>(&pluginProcessorParameters)
	);
	~MultiBandCompressor();

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	float getLatency();

private:
	// TODO: abstract the class from the processor stage of the plugin so that it can be used separately
	//==============================================================================
	// --- Object parameters management and information
	const std::set<ControlID> multiBandCompressorIDs = {
		// -- Low-mid crossover
		ControlID::lowMidCrossoverFreq,
		// -- Mid-high crossover
		ControlID::midHighCrossoverFreq
	};

	std::array<ParameterDefinition, ControlID::countParams>(*parameterDefinitions) { nullptr };
	std::array<ParameterObject, ControlID::countParams>(*pluginProcessorParameters) { nullptr };

	//==============================================================================
	// --- Object member variables

	// -- Band filters
	float lowMidCrossoverFreq{ 0.f };
	float midHighCrossoverFreq{ 0.f };

	//==============================================================================
	// -- Band filters
	using Filter = juce::dsp::LinkwitzRileyFilter<float>;

	// -- Low band filter -- lowpass1 -> allpass = lowpass band (----\) -- we add an allpass to avoid phase issues, since each filter adds a small delay and we need 2 filters for the mid crossover
	// -- mid band filter -- highpass1 -> lowpass2 = mid band (/---\)
	// -- high band filter -- highpass1 -> highpass2 = highpass band (/----)
	enum FilterIDs
	{
		lowpass1,
		allpass,

		highpass1,
		lowpass2,

		highpass2,
		// -- Count
		countFilters
	};
	std::array<Filter, FilterIDs::countFilters> bandFilters;

	// -- 0: low band, 1: mid band, 2: high band
	enum BandIDs
	{
		lowBand,
		midBand,
		highBand,
		// -- Count
		countBands
	};
	std::array<juce::AudioBuffer<float>, BandIDs::countBands> filterBuffers;
	std::array<juce::dsp::AudioBlock<float>, BandIDs::countBands> filterBlocks;
	std::array<CompressorBand, 3> compressorBands;

	//==============================================================================
	void preProcess();

	void syncInBoundVariables();
	void postUpdatePluginParameters();
};