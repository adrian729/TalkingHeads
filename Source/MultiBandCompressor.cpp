/*
  ==============================================================================

	MultiBandCompressor.cpp
	Created: 22 Sep 2023 5:01:45pm
	Author:  Brutus729

  ==============================================================================
*/

#include "MultiBandCompressor.h"

//==============================================================================
// -- CONSTRUCTORS
//==============================================================================
MultiBandCompressor::MultiBandCompressor()
{
}

MultiBandCompressor::~MultiBandCompressor()
{
}

//==============================================================================
CompressorBand* MultiBandCompressor::getLowBandCompressor()
{
	return &compressorBands[BandIDs::lowBand];
}

CompressorBand* MultiBandCompressor::getMidBandCompressor()
{
	return &compressorBands[BandIDs::midBand];
}

CompressorBand* MultiBandCompressor::getHighBandCompressor()
{
	return &compressorBands[BandIDs::highBand];
}

//==============================================================================
void MultiBandCompressor::prepare(const juce::dsp::ProcessSpec& spec)
{
	auto sampleRate = spec.sampleRate;

	// -- Filter buffers
	for (juce::AudioBuffer<float>& buffer : filterBuffers)
	{
		buffer.setSize(spec.numChannels, spec.maximumBlockSize);
		buffer.clear();
	}

	// -- Compressor bands
	for (CompressorBand& band : compressorBands)
	{
		band.prepare(spec);
	}
}

void MultiBandCompressor::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	// -- Get context for each band
	juce::dsp::AudioBlock<const float> inputBlock = context.getInputBlock();
	juce::dsp::AudioBlock<float> outputBlock = context.getOutputBlock();

	for (juce::AudioBuffer<float>& buffer : filterBuffers)
	{
		for (int channel{ 0 }; channel < inputBlock.getNumChannels(); ++channel)
		{
			buffer.copyFrom(channel, 0, inputBlock.getChannelPointer(channel), buffer.getNumSamples());
		}
	}

	for (int i{ 0 }; i < BandIDs::countBands; ++i)
	{
		filterBlocks[i] = juce::dsp::AudioBlock<float>(filterBuffers[i]);
	}

	std::array<juce::dsp::ProcessContextReplacing<float>, BandIDs::countBands> bandContexts = {
		juce::dsp::ProcessContextReplacing<float>(filterBlocks[BandIDs::lowBand]),
		juce::dsp::ProcessContextReplacing<float>(filterBlocks[BandIDs::midBand]),
		juce::dsp::ProcessContextReplacing<float>(filterBlocks[BandIDs::highBand])
	};

	// -- Process each band
	for (int i{ 0 }; i < BandIDs::countBands; ++i)
	{
		compressorBands[i].process(bandContexts[i]);
	}

	// -- add all bands to output block
	outputBlock.clear();
	for (auto& filterBlock : filterBlocks)
	{
		outputBlock.add(filterBlock);
	}
}

void MultiBandCompressor::reset()
{
	for (CompressorBand& band : compressorBands)
	{
		band.reset();
	}
}

//==============================================================================
float MultiBandCompressor::getLatency()
{
	float latency = 0.f;
	for (CompressorBand& band : compressorBands)
	{
		latency += band.getLatency();
	}

	return latency;
}
