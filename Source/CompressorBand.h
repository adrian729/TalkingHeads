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
#include "PluginStateManager.h"

class CompressorBand : public  juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	CompressorBand(
		std::shared_ptr<PluginStateManager> stateManager,
		// -- Compressor
		ControlID bypassID,
		ControlID thresholdID,
		ControlID attackID,
		ControlID releaseID,
		ControlID ratioID,
		// -- Crossover filters
		ControlID firstFilterCrossoverFreqID,
		ControlID secondFilterCrossoverFreqID,
		juce::dsp::LinkwitzRileyFilterType firstFilterType,
		juce::dsp::LinkwitzRileyFilterType secondFilterType
	);
	~CompressorBand();

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	float getLatency();

private:
	//==============================================================================
	// --- Object parameters management and information
	std::shared_ptr<PluginStateManager> stateManager;

	// -- Filters
	enum FilterIDs
	{
		firstFilter,
		secondFilter,
		// -- count
		countFilters
	};
	std::array < ControlID, FilterIDs::countFilters> filterCrossoverFreqIDs;

	// -- Compressor
	ControlID bypassID{ ControlID::countParams };
	ControlID thresholdID{ ControlID::countParams };
	ControlID attackID{ ControlID::countParams };
	ControlID releaseID{ ControlID::countParams };
	ControlID ratioID{ ControlID::countParams };

	//==============================================================================
	// --- Object member variables

	// -- Filters


	std::array<float, FilterIDs::countFilters> crossoverFreqs{ 0.f };

	using Filter = juce::dsp::LinkwitzRileyFilter<float>;
	std::array<Filter, FilterIDs::countFilters> filters;

	// -- Compressor
	float bypass{ 0.f };
	bool isBypassed{ false };
	float threshold{ 0.f };
	float attack{ 0.f };
	float release{ 0.f };
	float ratio{ 0.f };

	juce::dsp::Compressor<float> compressor;

	//==============================================================================
	void prepareFilters(const juce::dsp::ProcessSpec& spec);
	void prepareCompressor(const juce::dsp::ProcessSpec& spec);
	//==============================================================================
	void preProcess();
	void preProcessFilters();
	void preProcessCompressor();
};