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
	parameterDefinitions = nullptr;
	pluginProcessorParameters = nullptr;
}

//==============================================================================
void MultiBandCompressor::setupMultiBandCompressor(
	std::array<ParameterDefinition, ControlID::countParams>& parameterDefinitions,
	std::array<ParameterObject, ControlID::countParams>& pluginProcessorParameters,
	ControlID lowMidCrossoverFreqID,
	ControlID midHighCrossoverFreqID
)
{
	this->parameterDefinitions = &parameterDefinitions;
	this->pluginProcessorParameters = &pluginProcessorParameters;
	this->lowMidCrossoverFreqID = lowMidCrossoverFreqID;
	this->midHighCrossoverFreqID = midHighCrossoverFreqID;
	controlIDs = { lowMidCrossoverFreqID, midHighCrossoverFreqID };
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

	std::array<juce::dsp::ProcessContextReplacing<float>, BandIDs::countBands> filterContexts = {
		juce::dsp::ProcessContextReplacing<float>(filterBlocks[BandIDs::lowBand]),
		juce::dsp::ProcessContextReplacing<float>(filterBlocks[BandIDs::midBand]),
		juce::dsp::ProcessContextReplacing<float>(filterBlocks[BandIDs::highBand])
	};

	// -- Low Band Filter
	bandFilters[FilterIDs::lowpass1].process(filterContexts[BandIDs::lowBand]);
	bandFilters[FilterIDs::allpass].process(filterContexts[BandIDs::lowBand]);

	// -- Mid Band/High Band -- Highpass1
	bandFilters[FilterIDs::highpass1].process(filterContexts[BandIDs::midBand]);
	filterBuffers[BandIDs::highBand] = filterBuffers[BandIDs::midBand]; // -- copy midBandBuffer with highpass1 to highBandBuffer
	// -- Mid Band -- Highpass1 -> Lowpass2
	bandFilters[FilterIDs::lowpass2].process(filterContexts[BandIDs::midBand]);
	// -- High Band -- Highpass1 -> Highpass2
	bandFilters[FilterIDs::highpass2].process(filterContexts[BandIDs::highBand]);

	for (int i{ 0 }; i < BandIDs::countBands; ++i)
	{
		compressorBands[i].process(filterContexts[i]);
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
	for (ControlID id : controlIDs)
	{
		(*pluginProcessorParameters)[id].updateInBoundVariable();
	}

	postUpdatePluginParameters();
}

void MultiBandCompressor::postUpdatePluginParameters()
{
	// -- Low-Mid
	float newLowMidCrossoverFreq = (*pluginProcessorParameters)[ControlID::lowMidCrossoverFreq].getNextValue();
	if (!juce::approximatelyEqual(newLowMidCrossoverFreq, lowMidCrossoverFreq))
	{
		lowMidCrossoverFreq = newLowMidCrossoverFreq;
		bandFilters[FilterIDs::lowpass1].setCutoffFrequency(lowMidCrossoverFreq);
		bandFilters[FilterIDs::highpass1].setCutoffFrequency(lowMidCrossoverFreq);
	}

	// -- Mid-High
	float newMidHighCrossoverFreq = (*pluginProcessorParameters)[ControlID::midHighCrossoverFreq].getNextValue();
	if (!juce::approximatelyEqual(newMidHighCrossoverFreq, midHighCrossoverFreq))
	{
		midHighCrossoverFreq = newMidHighCrossoverFreq;
		bandFilters[FilterIDs::allpass].setCutoffFrequency(midHighCrossoverFreq);
		bandFilters[FilterIDs::lowpass2].setCutoffFrequency(midHighCrossoverFreq);
		bandFilters[FilterIDs::highpass2].setCutoffFrequency(midHighCrossoverFreq);
	}
}
