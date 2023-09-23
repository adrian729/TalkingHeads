/*
  ==============================================================================

	CompressorBand.cpp
	Created: 23 Sep 2023 1:20:20pm
	Author:  Brutus729

  ==============================================================================
*/

#include <cmath>
#include "CompressorBand.h"

//==============================================================================
// -- CONSTRUCTORS
//==============================================================================
CompressorBand::CompressorBand()
{
}

CompressorBand::~CompressorBand()
{
	parameterDefinitions = nullptr;
	pluginProcessorParameters = nullptr;
}

//==============================================================================
void CompressorBand::prepare(const juce::dsp::ProcessSpec& spec)
{
	auto sampleRate = spec.sampleRate;

	bypass = (*pluginProcessorParameters)[bypassID].getBoolValue();
	compressor.setThreshold((*pluginProcessorParameters)[thresholdID].getFloatValue());
	compressor.setAttack((*pluginProcessorParameters)[attackID].getFloatValue());
	compressor.setRelease((*pluginProcessorParameters)[releaseID].getFloatValue());
	compressor.setRatio((*pluginProcessorParameters)[ratioID].getFloatValue());

	compressor.prepare(spec);

	// TODO: check smoothings are working, it makes noises
	// -- Setup smoothing
	(*pluginProcessorParameters)[bypassID].initSmoothing(sampleRate, 0.01f);
	(*pluginProcessorParameters)[thresholdID].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[attackID].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[releaseID].initSmoothing(sampleRate);
	(*pluginProcessorParameters)[ratioID].initSmoothing(sampleRate);
}

void CompressorBand::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	preProcess();

	if (!juce::approximatelyEqual(bypass, 1.f))
	{
		compressor.process(context);
	}
}

void CompressorBand::reset()
{
	compressor.reset();
}

//==============================================================================
float CompressorBand::getLatency()
{
	return 0.f;
}

//==============================================================================
void CompressorBand::setupCompressorBand(
	std::array<ParameterDefinition, ControlID::countParams>& parameterDefinitions,
	std::array<ParameterObject, ControlID::countParams>& pluginProcessorParameters,
	ControlID bypassID,
	ControlID thresholdID,
	ControlID attackID,
	ControlID releaseID,
	ControlID ratioID
)
{
	this->parameterDefinitions = &parameterDefinitions;
	this->pluginProcessorParameters = &pluginProcessorParameters;
	this->bypassID = bypassID;
	this->thresholdID = thresholdID;
	this->attackID = attackID;
	this->releaseID = releaseID;
	this->ratioID = ratioID;
	this->compressorBandControlIDs = { bypassID, thresholdID, attackID, releaseID, ratioID };
}

//==============================================================================
void CompressorBand::preProcess()
{
	syncInBoundVariables();
}

void CompressorBand::syncInBoundVariables()
{
	for (auto& id : compressorBandControlIDs)
	{
		(*pluginProcessorParameters)[id].updateInBoundVariable();
	}

	postUpdatePluginParameters();
}

void CompressorBand::postUpdatePluginParameters()
{
	bypass = (*pluginProcessorParameters)[bypassID].getNextValue();
	compressor.setThreshold((*pluginProcessorParameters)[thresholdID].getNextValue());
	compressor.setAttack((*pluginProcessorParameters)[attackID].getNextValue());
	compressor.setRelease((*pluginProcessorParameters)[releaseID].getNextValue());

	float ratio = (*pluginProcessorParameters)[ratioID].getNextValue();
	ratio = std::max(ratio * (1.f - bypass), 1.f);
	compressor.setRatio(ratio);
}
