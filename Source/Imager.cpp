#pragma once

#include "Imager.h"

//==============================================================================
// -- CONSTRUCTORS
//==============================================================================
Imager::Imager(
	std::shared_ptr<PluginStateManager> stateManager,
	ControlID bypassID,
	ControlID originalGainID,
	ControlID auxiliarGainID,
	ControlID widthID,
	ControlID centerID,
	ControlID delayTimeID
) :
	stateManager(stateManager),
	bypassID(bypassID),
	gainIDs{ originalGainID, auxiliarGainID },
	widthID(widthID),
	centerID(centerID),
	delayTimeID(delayTimeID)
{
}

Imager::~Imager()
{
}

//==============================================================================
void Imager::prepare(const juce::dsp::ProcessSpec& spec)
{
	sampleRate = spec.sampleRate;

	bypass = stateManager->getFloatValue(bypassID);
	isBypassed = juce::approximatelyEqual(bypass, 1.f);

	for (int i{ 0 }; i < SignalIDs::countSignals; i++)
	{
		gains[i] = stateManager->getFloatValue(gainIDs[i]);
	}

	width = stateManager->getFloatValue(widthID);
	center = stateManager->getFloatValue(centerID);

	delayTime = stateManager->getFloatValue(delayTimeID);
	stereoImagerDelayLine.setDelay(getDelayTimeInSamples()); // -- delay time in samples (sampleRate * time in s)
	stereoImagerDelayLine.prepare(spec);
}

void Imager::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	const auto& inputBlock = context.getInputBlock();
	auto& outputBlock = context.getOutputBlock();
	int numSamples = outputBlock.getNumSamples();

	const int numOutputChannels = context.getOutputBlock().getNumChannels();
	if (numOutputChannels < 2)
	{
		return;
	}

	preProcess();

	for (int channel{ 2 }; channel < numOutputChannels; ++channel)
	{
		outputBlock.getSingleChannelBlock(channel).clear();
	}

	if (isBypassed)
	{
		outputBlock.getSingleChannelBlock(LEFT_CHANNEL).copyFrom(inputBlock);
		outputBlock.getSingleChannelBlock(RIGHT_CHANNEL).copyFrom(inputBlock);
		return;
	}

	/*
	* TODO:
	* If isBypassed, copy input mono context into channels 0 and 1 of output context (mono to stereo). Check if gain needs to be adjusted (not sure if it needs to be 0.5 for each channel or 1 each)
	*  - Copy context into two (original and auxiliar)
	*  - Delay aux signal with delay line
	*  - Apply its gain to each context
	*  - Calculate panning value for each context with width and center
	*  - Get output for left and right channels with mix of panned original and auxiliar signals
	*/

	const float* inSamples = inputBlock.getChannelPointer(MONO_CHANNEL);
	float* leftOutSamples = outputBlock.getChannelPointer(LEFT_CHANNEL);
	float* rightOutSamples = outputBlock.getChannelPointer(RIGHT_CHANNEL);

	// TODO: check if doing 0.5f origin sample to both channels and then invert and change panning of aux signal or do original and aux signals panned as it is done now
	// TODO: add bands to Imager (low and high), usually we want to keep low frequencies centered and high frequencies more wide
	for (int i{ 0 }; i < numSamples; i++)
	{
		stereoImagerDelayLine.pushSample(MONO_CHANNEL, inSamples[i]);

		float originalSample = inSamples[i] * gains[SignalIDs::original];

		float delayedSample = stereoImagerDelayLine.popSample(MONO_CHANNEL);
		float auxSample = delayedSample * gains[SignalIDs::original];

		leftOutSamples[i] = originalSample * (1.f - panningCoefficients[SignalIDs::original]) + auxSample * (1.f - panningCoefficients[SignalIDs::auxiliar]);
		rightOutSamples[i] = originalSample * panningCoefficients[SignalIDs::original] + auxSample * panningCoefficients[SignalIDs::auxiliar];
	}
}

void Imager::reset()
{
	stereoImagerDelayLine.reset();
}

//==============================================================================
float Imager::getLatency()
{
	// TODO: implement and check latency
	return 0.0f;
}

//==============================================================================
void Imager::setSampleRate(int newSampleRate)
{
	sampleRate = newSampleRate;
}

//==============================================================================
float Imager::getDelayTimeInSamples()
{
	return sampleRate * delayTime / 1000.f;
}

//==============================================================================
void Imager::preProcess()
{
	float newBypass = stateManager->getCurrentValue(bypassID);
	bool bypassChanged = !juce::approximatelyEqual(newBypass, bypass);
	bypass = newBypass;

	isBypassed = juce::approximatelyEqual(bypass, 1.0f);
	if (isBypassed)
	{
		return;
	}

	// TODO: check how to ramp gain to +0dB when bypassing changes
	for (int i{ 0 }; i < SignalIDs::countSignals; i++)
	{
		gains[i] = stateManager->getCurrentValue(gainIDs[i]);
	}

	width = stateManager->getCurrentValue(widthID);
	center = stateManager->getCurrentValue(centerID);

	float newDelayTime = stateManager->getCurrentValue(delayTimeID);
	if (!juce::approximatelyEqual(newDelayTime, delayTime) || bypassChanged)
	{
		delayTime = newDelayTime;
		stereoImagerDelayLine.setDelay((1.f - bypass) * getDelayTimeInSamples());
	}

	calculatePanningCoefficients();
}

void Imager::calculatePanningCoefficients()
{
	// -- Panning distance between original/auxiliar and center. original panned to -width and auxiliar to +width
	// -- Panning center displacement
	// --  f.e. If center at 0 and width at 0.5, original signal will be panned to -0.5 and auxiliar to 0.5 (range L[-1, 1]R), or 0.25 and 0.75 (range L[0, 1]R)
	// -- f.e. If center at 0.2 and width 0.6, original signal will be panned at -0.4 and aux to 0.8 (range L[-1, 1]R), or 0.3 and 0.9 (range L[0, 1]R)

	// -- Panning coefficients -- range L[0, 1]R 0 full left, 1 full right, 0.5 center
	panningCoefficients[SignalIDs::original] = (juce::jlimit(-1.f, 1.f, center - width) + 1.f) * .5f;
	panningCoefficients[SignalIDs::auxiliar] = (juce::jlimit(-1.f, 1.f, center + width) + 1.f) * .5f;
}