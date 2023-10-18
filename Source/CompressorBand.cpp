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
CompressorBand::CompressorBand(
	std::shared_ptr<PluginStateManager> stateManager,
	ControlID muteID,
	// -- Compressor
	ControlID bypassID,
	ControlID thresholdID,
	ControlID attackID,
	ControlID releaseID,
	ControlID ratioID,
	// -- Crossover filters
	ControlID firstFilterCrossoverFreqID,
	ControlID secondFilterCrossoverFreqID,
	juce::dsp::LinkwitzRileyFilterType firstFilterType,
	juce::dsp::LinkwitzRileyFilterType secondFilterType
) :
	stateManager(stateManager),
	muteID(muteID),
	// -- Compressor
	bypassID(bypassID),
	thresholdID(thresholdID),
	attackID(attackID),
	releaseID(releaseID),
	ratioID(ratioID),
	// -- Filters
	filterCrossoverFreqIDs{ firstFilterCrossoverFreqID, secondFilterCrossoverFreqID }
{
	filters[FilterIDs::firstFilter].setType(firstFilterType);
	filters[FilterIDs::secondFilter].setType(secondFilterType);
}

CompressorBand::~CompressorBand()
{
}

//==============================================================================
void CompressorBand::prepare(const juce::dsp::ProcessSpec& spec)
{
	mute = stateManager->getFloatValue(muteID);
	isMuted = juce::approximatelyEqual(mute, 1.f);
	prepareFilters(spec);
	prepareCompressor(spec);
}

void CompressorBand::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	preProcess();

	if (isMuted)
	{
		context.getOutputBlock().clear();
		return;
	}

	// -- Filters -- even if we bypass, we need to let pass only the band part of the audio
	for (auto& filter : filters)
	{
		filter.process(context);
	}
	if (!isBypassed)
	{
		compressor.process(context);
	}
}

void CompressorBand::reset()
{
	for (auto& filter : filters)
	{
		filter.reset();
	}
	compressor.reset();
}

//==============================================================================
float CompressorBand::getLatency()
{
	return 0.f; // TODO: check if filters or compressor add latency
}

//==============================================================================
void CompressorBand::prepareFilters(const juce::dsp::ProcessSpec& spec)
{
	for (int i{ 0 }; i < FilterIDs::countFilters; ++i)
	{
		crossoverFreqs[i] = 500.f;
		if (filters[i].getType() != juce::dsp::LinkwitzRileyFilterType::allpass)
		{
			crossoverFreqs[i] = stateManager->getFloatValue(filterCrossoverFreqIDs[i]);
		}
		filters[i].setCutoffFrequency(crossoverFreqs[i]);
		filters[i].prepare(spec);
	}
}

void CompressorBand::prepareCompressor(const juce::dsp::ProcessSpec& spec)
{
	bypass = stateManager->getFloatValue(bypassID);
	isBypassed = juce::approximatelyEqual(bypass, 1.f);
	compressor.setThreshold(stateManager->getFloatValue(thresholdID));
	compressor.setAttack(stateManager->getFloatValue(attackID));
	compressor.setRelease(stateManager->getFloatValue(releaseID));
	compressor.setRatio(stateManager->getFloatValue(ratioID));
	compressor.prepare(spec);
}

//==============================================================================
void CompressorBand::preProcess()
{
	// -- Mute
	float newMute = stateManager->getCurrentValue(muteID);
	if (!juce::approximatelyEqual(newMute, mute))
	{
		mute = newMute;
		isMuted = juce::approximatelyEqual(mute, 1.f);
	}

	if (isMuted)
	{
		return;
	}

	preProcessFilters();
	preProcessCompressor();
}

void CompressorBand::preProcessFilters()
{
	for (int i{ 0 }; i < FilterIDs::countFilters; ++i)
	{
		if (filters[i].getType() != juce::dsp::LinkwitzRileyFilterType::allpass)
		{
			float newCrossoverFreq = stateManager->getCurrentValue(filterCrossoverFreqIDs[i]);
			if (!juce::approximatelyEqual(newCrossoverFreq, crossoverFreqs[i]))
			{
				crossoverFreqs[i] = newCrossoverFreq;
				filters[i].setCutoffFrequency(crossoverFreqs[i]);
			}
		}
	}
}

void CompressorBand::preProcessCompressor()
{
	// -- Bypass
	float newBypass = stateManager->getCurrentValue(bypassID);
	bool bypassChanged = !juce::approximatelyEqual(newBypass, bypass);
	if (bypassChanged)
	{
		bypass = newBypass;
		isBypassed = juce::approximatelyEqual(bypass, 1.f);
	}

	if (isBypassed)
	{
		return;
	}

	// -- Threshold
	float newThreshold = stateManager->getCurrentValue(thresholdID);
	if (!juce::approximatelyEqual(newThreshold, threshold))
	{
		threshold = newThreshold;
		compressor.setThreshold(threshold);
	}

	// -- Attack
	float newAttack = stateManager->getCurrentValue(attackID);
	if (!juce::approximatelyEqual(newAttack, attack))
	{
		attack = newAttack;
		compressor.setAttack(attack);
	}

	// -- Release
	float newRelease = stateManager->getCurrentValue(releaseID);
	if (!juce::approximatelyEqual(newRelease, release))
	{
		release = newRelease;
		compressor.setRelease(release);
	}

	// -- Ratio -- when bypassing we smooth the ratio to 1.f
	float newRatio = stateManager->getCurrentValue(ratioID);
	if (!juce::approximatelyEqual(newRatio, ratio) || bypassChanged)
	{
		ratio = newRatio;
		compressor.setRatio(juce::jmax(1.f, ratio * (1.f - bypass)));
	}
}
