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
	MultiBandCompressor();
	~MultiBandCompressor();

	//==============================================================================
	CompressorBand* getLowBandCompressor();
	CompressorBand* getMidBandCompressor();
	CompressorBand* getHighBandCompressor();

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	float getLatency();

private:
	//==============================================================================
	// --- Object member variables
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
	std::array<CompressorBand, BandIDs::countBands> compressorBands;

};