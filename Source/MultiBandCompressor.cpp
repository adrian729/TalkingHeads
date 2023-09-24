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
	std::array<ParameterDefinition, ControlID::countParams>(&parameterDefinitions),
	std::array<ParameterObject, ControlID::countParams>(&pluginProcessorParameters)
) :
	parameterDefinitions(&parameterDefinitions),
	pluginProcessorParameters(&pluginProcessorParameters)
{
	// -- Low Band Compressor
	compressorBands[BandIDs::lowBand].setupCompressorBand(
		parameterDefinitions,
		pluginProcessorParameters,
		ControlID::lowBandCompressorBypass,
		ControlID::lowBandCompressorThreshold,
		ControlID::lowBandCompressorAttack,
		ControlID::lowBandCompressorRelease,
		ControlID::lowBandCompressorRatio
	);

	// -- Mid Band Compressor
	compressorBands[BandIDs::midBand].setupCompressorBand(
		parameterDefinitions,
		pluginProcessorParameters,
		ControlID::midBandCompressorBypass,
		ControlID::midBandCompressorThreshold,
		ControlID::midBandCompressorAttack,
		ControlID::midBandCompressorRelease,
		ControlID::midBandCompressorRatio
	);

	// -- High Band Compressor
	compressorBands[BandIDs::highBand].setupCompressorBand(
		parameterDefinitions,
		pluginProcessorParameters,
		ControlID::highBandCompressorBypass,
		ControlID::highBandCompressorThreshold,
		ControlID::highBandCompressorAttack,
		ControlID::highBandCompressorRelease,
		ControlID::highBandCompressorRatio
	);
}

MultiBandCompressor::~MultiBandCompressor()
{
	parameterDefinitions = nullptr;
	pluginProcessorParameters = nullptr;
}

//==============================================================================
void MultiBandCompressor::prepare(const juce::dsp::ProcessSpec& spec)
{
	auto sampleRate = spec.sampleRate;

	// -- Band Filters
	// -- Low-Mid
	lowMidCrossoverFreq = (*pluginProcessorParameters)[ControlID::lowMidCrossoverFreq].getFloatValue();

	bandFilters[FilterIDs::lowpass1].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	bandFilters[FilterIDs::lowpass1].setCutoffFrequency(lowMidCrossoverFreq);

	bandFilters[FilterIDs::highpass1].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
	bandFilters[FilterIDs::highpass1].setCutoffFrequency(lowMidCrossoverFreq);

	// -- Mid-High
	midHighCrossoverFreq = (*pluginProcessorParameters)[ControlID::midHighCrossoverFreq].getFloatValue();

	bandFilters[FilterIDs::allpass].setType(juce::dsp::LinkwitzRileyFilterType::allpass);
	bandFilters[FilterIDs::allpass].setCutoffFrequency(midHighCrossoverFreq);

	bandFilters[FilterIDs::lowpass2].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	bandFilters[FilterIDs::lowpass2].setCutoffFrequency(midHighCrossoverFreq);

	bandFilters[FilterIDs::highpass2].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
	bandFilters[FilterIDs::highpass2].setCutoffFrequency(midHighCrossoverFreq);

	for (auto& filter : bandFilters)
	{
		filter.prepare(spec);
	}

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

	// -- Setup smoothing
	(*pluginProcessorParameters)[ControlID::lowMidCrossoverFreq].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::midHighCrossoverFreq].initSmoothing(sampleRate);
}

void MultiBandCompressor::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	preProcess();

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

	juce::dsp::ProcessContextReplacing<float> lowBandContext(filterBlocks[BandIDs::lowBand]);
	juce::dsp::ProcessContextReplacing<float> midBandContext(filterBlocks[BandIDs::midBand]);
	juce::dsp::ProcessContextReplacing<float> highBandContext(filterBlocks[BandIDs::highBand]);

	// -- Low Band Filter
	bandFilters[FilterIDs::lowpass1].process(lowBandContext);
	bandFilters[FilterIDs::allpass].process(lowBandContext);

	// -- Mid Band/High Band Filter
	bandFilters[FilterIDs::highpass1].process(midBandContext);
	filterBuffers[2] = filterBuffers[1]; // -- copy midBandBuffer with highpass1 to highBandBuffer
	bandFilters[FilterIDs::lowpass2].process(midBandContext);
	bandFilters[FilterIDs::highpass2].process(highBandContext);

	for (int i{ 0 }; i < BandIDs::countBands; ++i)
	{
		compressorBands[i].process(filterBlocks[i]);
	}

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

//==============================================================================
void MultiBandCompressor::preProcess()
{
	syncInBoundVariables();
}

void MultiBandCompressor::syncInBoundVariables()
{
	for (ControlID id : multiBandCompressorIDs)
	{
		(*pluginProcessorParameters)[id].updateInBoundVariable();
	}

	postUpdatePluginParameters();
}

void MultiBandCompressor::postUpdatePluginParameters()
{
	// -- Low-Mid
	lowMidCrossoverFreq = (*pluginProcessorParameters)[ControlID::lowMidCrossoverFreq].getNextValue();

	bandFilters[FilterIDs::lowpass1].setCutoffFrequency(lowMidCrossoverFreq);
	bandFilters[FilterIDs::highpass1].setCutoffFrequency(lowMidCrossoverFreq);

	// -- Mid-High
	midHighCrossoverFreq = (*pluginProcessorParameters)[ControlID::midHighCrossoverFreq].getNextValue();

	bandFilters[FilterIDs::allpass].setCutoffFrequency(midHighCrossoverFreq);
	bandFilters[FilterIDs::lowpass2].setCutoffFrequency(midHighCrossoverFreq);
	bandFilters[FilterIDs::highpass2].setCutoffFrequency(midHighCrossoverFreq);
}
