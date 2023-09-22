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

	// -- Low Band Compressor
	auto& compressor = processorChain.template get<lowBandCompressorChainIndex>();
	compressor.setThreshold((*pluginProcessorParameters)[ControlID::lowBandCompressorThreshold].getNextValue());
	compressor.setAttack((*pluginProcessorParameters)[ControlID::lowBandCompressorAttack].getNextValue());
	compressor.setRelease((*pluginProcessorParameters)[ControlID::lowBandCompressorRelease].getNextValue());
	compressor.setRatio((*pluginProcessorParameters)[ControlID::lowBandCompressorRatio].getNextValue());

	// -- Chain
	processorChain.prepare(spec);

	// TODO: check smoothings are working, it makes noises
	// -- Setup smoothing
	(*pluginProcessorParameters)[ControlID::lowBandCompressorThreshold].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::lowBandCompressorAttack].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::lowBandCompressorRelease].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ControlID::lowBandCompressorRatio].initSmoothing(sampleRate);
}

void MultiBandCompressor::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	preProcess();

	processorChain.process(context);

	postProcess();
}

void MultiBandCompressor::reset()
{
	processorChain.reset();
}

//==============================================================================
float MultiBandCompressor::getLatency()
{
	return 0.f;
}

//==============================================================================
void MultiBandCompressor::preProcess()
{
	syncInBoundVariables();
}

void MultiBandCompressor::syncInBoundVariables()
{
	for (auto& id : firstStageControlIDs)
	{
		(*pluginProcessorParameters)[id].updateInBoundVariable();
		postUpdatePluginParameter(id);
	}
}

bool MultiBandCompressor::postUpdatePluginParameter(ControlID controlID)
{
	switch (controlID)
	{
		// -- Low Band Compressor
	case ControlID::lowBandCompressorBypass:
	{
		processorChain.template setBypassed<lowBandCompressorChainIndex>((*pluginProcessorParameters)[controlID].getBoolValue());
		break;
	}
	case ControlID::lowBandCompressorThreshold:
	{
		auto& compressor = processorChain.template get<lowBandCompressorChainIndex>();
		compressor.setThreshold((*pluginProcessorParameters)[controlID].getNextValue());
		break;
	}
	case ControlID::lowBandCompressorAttack:
	{
		auto& compressor = processorChain.template get<lowBandCompressorChainIndex>();
		compressor.setAttack((*pluginProcessorParameters)[controlID].getNextValue());
		break;
	}
	case ControlID::lowBandCompressorRelease:
	{
		auto& compressor = processorChain.template get<lowBandCompressorChainIndex>();
		compressor.setRelease((*pluginProcessorParameters)[controlID].getNextValue());
		break;
	}
	case ControlID::lowBandCompressorRatio:
	{
		auto& compressor = processorChain.template get<lowBandCompressorChainIndex>();
		compressor.setRatio((*pluginProcessorParameters)[controlID].getNextValue());
		break;
	}

	default:
		return false;
	}

	return true;
}

void MultiBandCompressor::postProcess()
{
}