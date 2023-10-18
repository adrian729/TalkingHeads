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
MultiBandCompressor::MultiBandCompressor(
	std::shared_ptr<PluginStateManager> stateManager,
	ControlID bypassID,
	ControlID crossoverLowMidID,
	ControlID crossoverMidHighID,
	CompressorBandParamIDs lowBandParamIDs,
	CompressorBandParamIDs midBandParamIDs,
	CompressorBandParamIDs highBandParamIDs
) :
	stateManager(stateManager),
	bypassID(bypassID),
	compressorBands{
		CompressorBand(
			stateManager,
			lowBandParamIDs.muteID,
			// -- Compressor
			lowBandParamIDs.bypassID,
			lowBandParamIDs.thresholdID,
			lowBandParamIDs.attackID,
			lowBandParamIDs.releaseID,
			lowBandParamIDs.ratioID,
			// -- Crossover filters
			crossoverLowMidID,
			ControlID::countParams,
			juce::dsp::LinkwitzRileyFilterType::lowpass,
			juce::dsp::LinkwitzRileyFilterType::allpass
		),
		CompressorBand(
			stateManager,
			midBandParamIDs.muteID,
			// -- Compressor
			midBandParamIDs.bypassID,
			midBandParamIDs.thresholdID,
			midBandParamIDs.attackID,
			midBandParamIDs.releaseID,
			midBandParamIDs.ratioID,
			// -- Crossover filters
			crossoverLowMidID,
			crossoverMidHighID,
			juce::dsp::LinkwitzRileyFilterType::highpass,
			juce::dsp::LinkwitzRileyFilterType::lowpass
		),
		CompressorBand(
			stateManager,
			highBandParamIDs.muteID,
			// -- Compressor
			highBandParamIDs.bypassID,
			highBandParamIDs.thresholdID,
			highBandParamIDs.attackID,
			highBandParamIDs.releaseID,
			highBandParamIDs.ratioID,
			// -- Crossover filters
			crossoverMidHighID,
			ControlID::countParams,
			juce::dsp::LinkwitzRileyFilterType::highpass,
			juce::dsp::LinkwitzRileyFilterType::allpass
		)
	}
{
}

MultiBandCompressor::~MultiBandCompressor()
{
}

//==============================================================================
void MultiBandCompressor::prepare(const juce::dsp::ProcessSpec& spec)
{
	bypass = stateManager->getFloatValue(bypassID);

	// -- Filter buffers
	for (juce::AudioBuffer<float>& buffer : filterBuffers)
	{
		buffer.setSize(spec.numChannels, spec.maximumBlockSize);
		buffer.clear();
	}

	// -- Compressor bands
	for (auto& band : compressorBands)
	{
		band.prepare(spec);
	}
}

void MultiBandCompressor::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	preProcess();

	if (juce::approximatelyEqual(bypass, 1.f))
	{
		return;
	}

	// -- Need to create a copy of the context for each band
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

	// -- Process each band
	for (int i{ 0 }; i < BandIDs::countBands; ++i)
	{
		juce::dsp::ProcessContextReplacing<float> bandContext(filterBlocks[i]);
		compressorBands[i].process(bandContext);
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
	for (auto& band : compressorBands)
	{
		band.reset();
	}
}

//==============================================================================
float MultiBandCompressor::getLatency()
{
	float latency = 0.f;
	for (auto& band : compressorBands)
	{
		latency += band.getLatency();
	}

	return latency;
}

//==============================================================================
void MultiBandCompressor::preProcess()
{
	bypass = stateManager->getCurrentValue(bypassID);
}