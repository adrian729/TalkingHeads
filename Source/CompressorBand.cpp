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
void CompressorBand::setupCompressorBand(
	std::array<ParameterDefinition, ControlID::countParams> &parameterDefinitions,
	std::array<ParameterObject, ControlID::countParams> &pluginProcessorParameters,
	// -- Compressor
	ControlID bypassID,
	ControlID thresholdID,
	ControlID attackID,
	ControlID releaseID,
	ControlID ratioID,
	// -- Filters
	ControlID firstFilterCrossoverFreqID,
	ControlID secondFilterCrossoverFreqID,
	juce::dsp::LinkwitzRileyFilterType firstFilterType,
	juce::dsp::LinkwitzRileyFilterType secondFilterType)
{
	this->parameterDefinitions = &parameterDefinitions;
	this->pluginProcessorParameters = &pluginProcessorParameters;
	// -- Compressor
	this->bypassID = bypassID;
	this->thresholdID = thresholdID;
	this->attackID = attackID;
	this->releaseID = releaseID;
	this->ratioID = ratioID;
	this->controlIDs = {bypassID, thresholdID, attackID, releaseID, ratioID};
	// -- Filters
	this->firstFilterCrossoverFreqID = firstFilterCrossoverFreqID;
	this->secondFilterCrossoverFreqID = secondFilterCrossoverFreqID;
	filters[FilterIDs::firstFilter].setType(firstFilterType);
	filters[FilterIDs::secondFilter].setType(secondFilterType);
}

//==============================================================================
void CompressorBand::prepare(const juce::dsp::ProcessSpec &spec)
{
	auto sampleRate = spec.sampleRate;

	// -- Band Filters
	// -- TODO: if we want smoothing for cutoff freqs we need to add it separately, as the variable is shared between bands
	if (firstFilterCrossoverFreqID != ControlID::countParams)
	{
		firstFilterCrossoverFreq = (*pluginProcessorParameters)[firstFilterCrossoverFreqID].getFloatValue();
		filters[FilterIDs::firstFilter].setCutoffFrequency(firstFilterCrossoverFreq);
	}

	if (secondFilterCrossoverFreqID != ControlID::countParams)
	{
		secondFilterCrossoverFreq = (*pluginProcessorParameters)[secondFilterCrossoverFreqID].getFloatValue();
		filters[FilterIDs::secondFilter].setCutoffFrequency(secondFilterCrossoverFreq);
	}

	for (auto &filter : filters)
	{
		filter.prepare(spec);
	}

	// -- Compressor
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

void CompressorBand::process(const juce::dsp::ProcessContextReplacing<float> &context)
{
	preProcess();

	if (!juce::approximatelyEqual(bypass, 1.f))
	{
		// -- Filters
		for (auto &filter : filters)
		{
			filter.process(context);
		}

		// -- Compressor
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
	return 0.f; // TODO: check if filters or compressor add latency
}

//==============================================================================
void CompressorBand::preProcess()
{
	syncInBoundVariables();
}

void CompressorBand::syncInBoundVariables()
{
	postUpdatePluginParameters();
}

void CompressorBand::postUpdatePluginParameters()
{
	// -- Filters
	// -- First filter
	if (firstFilterCrossoverFreqID != ControlID::countParams)
	{
		float newFirstFilterCrossoverFreq = (*pluginProcessorParameters)[firstFilterCrossoverFreqID].getFloatValue();
		if (!juce::approximatelyEqual(newFirstFilterCrossoverFreq, firstFilterCrossoverFreq))
		{
			firstFilterCrossoverFreq = newFirstFilterCrossoverFreq;
			filters[FilterIDs::firstFilter].setCutoffFrequency(firstFilterCrossoverFreq);
		}
	}
	// -- Second filter
	if (secondFilterCrossoverFreqID != ControlID::countParams)
	{
		float newSecondFilterCrossoverFreq = (*pluginProcessorParameters)[secondFilterCrossoverFreqID].getFloatValue();
		if (!juce::approximatelyEqual(newSecondFilterCrossoverFreq, secondFilterCrossoverFreq))
		{
			secondFilterCrossoverFreq = newSecondFilterCrossoverFreq;
			filters[FilterIDs::secondFilter].setCutoffFrequency(secondFilterCrossoverFreq);
		}
	}

	// -- Compressor
	// -- Bypass
	float newBypass = (*pluginProcessorParameters)[bypassID].getCurrentValue();
	bool bypassChanged = !juce::approximatelyEqual(newBypass, bypass);
	bypass = newBypass;

	// -- Threshold
	float newThreshold = (*pluginProcessorParameters)[thresholdID].getCurrentValue();
	if (!juce::approximatelyEqual(newThreshold, threshold))
	{
		threshold = newThreshold;
		compressor.setThreshold(threshold);
	}

	// -- Attack
	float newAttack = (*pluginProcessorParameters)[attackID].getCurrentValue();
	if (!juce::approximatelyEqual(newAttack, attack))
	{
		attack = newAttack;
		compressor.setAttack(attack);
	}

	// -- Release
	float newRelease = (*pluginProcessorParameters)[releaseID].getCurrentValue();
	if (!juce::approximatelyEqual(newRelease, release))
	{
		release = newRelease;
		compressor.setRelease(release);
	}

	// -- Ratio -- when bypassing we smooth the ratio to 1.f
	float newRatio = (*pluginProcessorParameters)[ratioID].getCurrentValue();
	bool ratioChanged = !juce::approximatelyEqual(newRatio, ratio);
	if (ratioChanged || bypassChanged)
	{
		ratio = std::max(newRatio * (1.f - bypass), 1.f);
		compressor.setRatio(ratio);
	}
}
