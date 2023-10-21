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
	ControlID delayTimeID,
	ControlID crossoverFreqID,
	ControlID imagerTypeID
) :
	stateManager(stateManager),
	bypassID(bypassID),
	gainIDs{ originalGainID, auxiliarGainID },
	widthID(widthID),
	centerID(centerID),
	delayTimeID(delayTimeID),
	crossoverFreqID(crossoverFreqID),
	imagerTypeID(imagerTypeID)
{
	filters[FilterIDs::lowpass].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	filters[FilterIDs::highpass].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
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

	crossoverFreq = stateManager->getFloatValue(crossoverFreqID);
	filters[FilterIDs::lowpass].setCutoffFrequency(crossoverFreq);
	filters[FilterIDs::highpass].setCutoffFrequency(crossoverFreq);
	filters[FilterIDs::lowpass].prepare(spec);
	filters[FilterIDs::highpass].prepare(spec);

	lowpassBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
	lowpassBuffer.clear();
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

	// -- Clear all channels except left and right
	for (int channel{ 2 }; channel < numOutputChannels; ++channel)
	{
		outputBlock.getSingleChannelBlock(channel).clear();
	}

	// -- If bypassed, copy mono input to left and right output
	if (isBypassed)
	{
		outputBlock.getSingleChannelBlock(LEFT_CHANNEL).copyFrom(inputBlock);
		outputBlock.getSingleChannelBlock(RIGHT_CHANNEL).copyFrom(inputBlock);
		return;
	}

	// -- Lowpass with original audio mono to stereo, and highpass with the imager processed signal
	lowpassBuffer.copyFrom(MONO_CHANNEL, 0, inputBlock.getChannelPointer(MONO_CHANNEL), numSamples); // TODO: buffer size is maxNumSamples, not numSamples. Check how we create audioBlocks with only numSamples
	lowpassBlock = juce::dsp::AudioBlock<float>(lowpassBuffer);

	filters[FilterIDs::lowpass].process(juce::dsp::ProcessContextReplacing<float>(lowpassBlock));
	filters[FilterIDs::highpass].process(context);

	// -- Process stereo imager and generate final stereo signal
	const float* inSamples = inputBlock.getChannelPointer(MONO_CHANNEL);
	const float* lowpassSamples = lowpassBlock.getChannelPointer(MONO_CHANNEL);
	float* leftOutSamples = outputBlock.getChannelPointer(LEFT_CHANNEL);
	float* rightOutSamples = outputBlock.getChannelPointer(RIGHT_CHANNEL);

	for (int i{ 0 }; i < numSamples; i++)
	{
		stereoImagerDelayLine.pushSample(MONO_CHANNEL, inSamples[i]);

		float originalSample = inSamples[i] * gains[SignalIDs::original];
		float auxSample = stereoImagerDelayLine.popSample(MONO_CHANNEL) * gains[SignalIDs::auxiliar];

		switch (imagerType)
		{
		case Imager::haas:
		{
			leftOutSamples[i] = originalSample * (1.f - panningCoefficients[SignalIDs::original]) + auxSample * (1.f - panningCoefficients[SignalIDs::auxiliar]);
			rightOutSamples[i] = originalSample * panningCoefficients[SignalIDs::original] + auxSample * panningCoefficients[SignalIDs::auxiliar];
			break;
		}
		case Imager::haasMono:
		{
			leftOutSamples[i] = originalSample + auxSample * panningCoefficients[StereoIDs::left];
			rightOutSamples[i] = originalSample + auxSample * panningCoefficients[StereoIDs::right];
			break;
		}
		case Imager::haasMidSide:
		{
			// TODO: check if we use centre for something or not
			float coef_S = width * 0.5;
			float mid = originalSample * 0.5;
			float side = auxSample * coef_S;

			leftOutSamples[i] = mid - side;
			rightOutSamples[i] = mid + side;
			break;
		}
		}

		// -- Add lowpass signal to both channels
		leftOutSamples[i] += lowpassSamples[i];
		rightOutSamples[i] += lowpassSamples[i];
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

	float newCrossoverFreq = stateManager->getCurrentValue(crossoverFreqID);
	if (!juce::approximatelyEqual(newCrossoverFreq, crossoverFreq))
	{
		crossoverFreq = newCrossoverFreq;
		filters[FilterIDs::lowpass].setCutoffFrequency(crossoverFreq);
		filters[FilterIDs::highpass].setCutoffFrequency(crossoverFreq);
	}

	imagerType = intToEnum(stateManager->getCurrentValue(imagerTypeID), ImagerTypes);

	calculatePanningCoefficients();
}

void Imager::calculatePanningCoefficients()
{

	switch (imagerType)
	{
	case Imager::haas:
	{
		// -- Panning distance between original/auxiliar and center. original panned to -width and auxiliar to +width
		// -- Panning center displacement
		// --  f.e. If center at 0 and width at 0.5, original signal will be panned to -0.5 and auxiliar to 0.5 (range L[-1, 1]R), or 0.25 and 0.75 (range L[0, 1]R)
		// -- f.e. If center at 0.2 and width 0.6, original signal will be panned at -0.4 and aux to 0.8 (range L[-1, 1]R), or 0.3 and 0.9 (range L[0, 1]R)

		// -- Panning coefficients -- range L[0, 1]R 0 full left, 1 full right, 0.5 center
		panningCoefficients[SignalIDs::original] = (juce::jlimit(-1.f, 1.f, center - width) + 1.f) * .5f;
		panningCoefficients[SignalIDs::auxiliar] = (juce::jlimit(-1.f, 1.f, center + width) + 1.f) * .5f;
		break;
	}
	case Imager::haasMono:
	{
		panningCoefficients[StereoIDs::left] = (center - width);
		panningCoefficients[StereoIDs::right] = (center + width);
		break;
	}
	case Imager::haasMidSide:
	{
		// TODO: implement or remove if not needed
		break;
	}
	}
}
