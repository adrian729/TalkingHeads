/*
  ==============================================================================

	MultiBandCompressor.h
	Created: 22 Sep 2023 5:01:45pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <memory>
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
	std::shared_ptr<CompressorBand> getLowBandCompressor();
	std::shared_ptr<CompressorBand> getMidBandCompressor();
	std::shared_ptr<CompressorBand> getHighBandCompressor();

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

	std::array<std::shared_ptr<CompressorBand>, BandIDs::countBands> compressorBands;

	// -- Helper variables
	std::array<juce::AudioBuffer<float>, BandIDs::countBands> filterBuffers;
	std::array<juce::dsp::AudioBlock<float>, BandIDs::countBands> filterBlocks;

};