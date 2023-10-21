/*
  ==============================================================================

	MultiBandCompressor.h
	Created: 22 Sep 2023 5:01:45pm
	Author:  Brutus729

	Compressor with multiple bands:
	-- Low band -- lowpass -> allpass = lowpass band (----\) -- we add an allpass to avoid phase issues
	-- Mid band -- highpass -> lowpass = mid band (/---\)
	-- High band -- highpass -> allpass = highpass band (/----)
	* allpass used to align phase of all bands
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <memory>
#include "parameterTypes.h"
#include "CompressorBand.h"
#include "PluginStateManager.h"

//==============================================================================
class MultiBandCompressor : public juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	struct CompressorBandParamIDs
	{
		ControlID muteID;
		ControlID bypassID;
		ControlID thresholdID;
		ControlID attackID;
		ControlID releaseID;
		ControlID ratioID;
	};

	MultiBandCompressor(
		std::shared_ptr<PluginStateManager> stateManager,
		ControlID bypassID,
		ControlID crossoverLowMidID,
		ControlID crossoverMidHighID,
		CompressorBandParamIDs lowBandParamIDs,
		CompressorBandParamIDs midBandParamIDs,
		CompressorBandParamIDs highBandParamIDs
	);
	~MultiBandCompressor();

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

	ControlID bypassID{ ControlID::countParams };

	// --- Object member variables
	float bypass{ 0.f };
	bool isBypassed{ false };

	static const int numBands = 3;
	std::array<CompressorBand, numBands> compressorBands;

	// -- Helper variables
	std::array<juce::AudioBuffer<float>, numBands> filterBuffers;
	std::array<juce::dsp::AudioBlock<float>, numBands> filterBlocks;

	//==============================================================================
	void preProcess();

};