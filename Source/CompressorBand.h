/*
  ==============================================================================

	CompressorBand.h
	Created: 23 Sep 2023 1:20:20pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "ParameterDefinition.h"
#include "ParameterObject.h"

class CompressorBand : public  juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	CompressorBand();
	~CompressorBand();

	//==============================================================================
	void setupCompressorBand(
		std::array<ParameterDefinition, ControlID::countParams>& parameterDefinitions,
		std::array<ParameterObject, ControlID::countParams>& pluginProcessorParameters,
		// -- Compressor
		ControlID bypassID,
		ControlID thresholdID,
		ControlID attackID,
		ControlID releaseID,
		ControlID ratioID,
		// -- Filters
		ControlID firstFilterCrossoverFreqID,
		ControlID secondFilterCrossoverFreqID,
		juce::dsp::LinkwitzRileyFilterType firstFilterType,
		juce::dsp::LinkwitzRileyFilterType secondFilterType
	);

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	float getLatency();

private:
	//==============================================================================
	// --- Object parameters management and information
	std::set<ControlID> controlIDs;

	ControlID bypassID{ ControlID::countParams };
	ControlID thresholdID{ ControlID::countParams };
	ControlID attackID{ ControlID::countParams };
	ControlID releaseID{ ControlID::countParams };
	ControlID ratioID{ ControlID::countParams };

	// -- if crossover freq ID == ControlID::countParams, that band is an allpass filter
	ControlID firstFilterCrossoverFreqID{ ControlID::countParams };
	ControlID secondFilterCrossoverFreqID{ ControlID::countParams };

	std::array<ParameterDefinition, ControlID::countParams>(*parameterDefinitions) { nullptr };
	std::array<ParameterObject, ControlID::countParams>(*pluginProcessorParameters) { nullptr };

	//==============================================================================
	// --- Object member variables

	// -- Filters
	float firstFilterCrossoverFreq{ 0.f };
	float secondFilterCrossoverFreq{ 0.f };

	enum FilterIDs
	{
		firstFilter,
		secondFilter,
		// -- count
		countFilters
	};
	using Filter = juce::dsp::LinkwitzRileyFilter<float>;
	std::array<Filter, FilterIDs::countFilters> filters;

	juce::AudioBuffer<float> filtersBuffer;
	juce::dsp::AudioBlock<float> filtersBlock;

	// -- Compressor
	float bypass{ 0.f };
	float threshold{ 0.f };
	float attack{ 0.f };
	float release{ 0.f };
	float ratio{ 0.f };

	juce::dsp::Compressor<float> compressor;

	//==============================================================================
	void preProcess();

	void syncInBoundVariables();
	void postUpdatePluginParameters();
};