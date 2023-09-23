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
	auto& lowBandCompressor = processorChain.template get<lowBandCompressorChainIndex>();
	lowBandCompressor.setupCompressorBand(
		parameterDefinitions,
		pluginProcessorParameters,
		ControlID::lowBandCompressorBypass,
		ControlID::lowBandCompressorThreshold,
		ControlID::lowBandCompressorAttack,
		ControlID::lowBandCompressorRelease,
		ControlID::lowBandCompressorRatio
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
	lowMidCrossoverFreq = (*pluginProcessorParameters)[ControlID::lowMidCrossoverFreq].getFloatValue();

	LP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	HP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

	LP.setCutoffFrequency(lowMidCrossoverFreq);
	HP.setCutoffFrequency(lowMidCrossoverFreq);

	LP.prepare(spec);
	HP.prepare(spec);

	for (juce::AudioBuffer<float>& buffer : filterBuffers)
	{
		buffer.setSize(spec.numChannels, spec.maximumBlockSize);
		buffer.clear();
	}

	// -- Low Band Compressor
	lowBandCompressorBypass = (*pluginProcessorParameters)[ControlID::lowBandCompressorBypass].getFloatValue();

	// -- Chain
	processorChain.prepare(spec);

	// -- bypass smoothing is already initialized in the CompressorBand
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

	juce::dsp::AudioBlock<float> lpBlock(filterBuffers[0]);
	juce::dsp::AudioBlock<float> hpBlock(filterBuffers[1]);
	juce::dsp::ProcessContextReplacing<float> lpContext(lpBlock);
	juce::dsp::ProcessContextReplacing<float> hpContext(hpBlock);

	LP.process(lpContext);
	HP.process(hpContext);

	outputBlock.clear();
	outputBlock.copyFrom(lpBlock);
	outputBlock.add(hpBlock);

	processorChain.process(context);
}

void MultiBandCompressor::reset()
{
	processorChain.reset();
}

//==============================================================================
float MultiBandCompressor::getLatency()
{
	float latency = 0.f;
	latency += processorChain.template get<lowBandCompressorChainIndex>().getLatency();

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
	// -- Band Filters
	lowMidCrossoverFreq = (*pluginProcessorParameters)[ControlID::lowMidCrossoverFreq].getNextValue();
	LP.setCutoffFrequency(lowMidCrossoverFreq);
	HP.setCutoffFrequency(lowMidCrossoverFreq);

	// -- Low Band Compressor
	lowBandCompressorBypass = (*pluginProcessorParameters)[ControlID::lowBandCompressorBypass].getNextValue();
	processorChain.template setBypassed<lowBandCompressorChainIndex>(juce::approximatelyEqual(lowBandCompressorBypass, 1.f));
}
