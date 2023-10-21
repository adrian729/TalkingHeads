/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TalkingHeadsPluginAudioProcessor::TalkingHeadsPluginAudioProcessor() :
	AudioProcessor(
		BusesProperties()
		.withInput("Input", juce::AudioChannelSet::mono(), true)
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
	),
	stateManager(std::make_shared<PluginStateManager>(
		PluginStateManager(*this, nullptr, juce::Identifier(APVTS_ID))
	)),
	multiBandCompressor(
		stateManager,
		ControlID::compressorBypass,
		// -- Crossover frequencies
		ControlID::lowMidCrossoverFreq,
		ControlID::midHighCrossoverFreq,
		// -- Compressor bands
		{
			ControlID::lowBandCompressorMute,
			ControlID::lowBandCompressorBypass,
			ControlID::lowBandCompressorThreshold,
			ControlID::lowBandCompressorAttack,
			ControlID::lowBandCompressorRelease,
			ControlID::lowBandCompressorRatio
		},
		{
			ControlID::midBandCompressorMute,
			ControlID::midBandCompressorBypass,
			ControlID::midBandCompressorThreshold,
			ControlID::midBandCompressorAttack,
			ControlID::midBandCompressorRelease,
			ControlID::midBandCompressorRatio
		},
		{
			ControlID::highBandCompressorMute,
			ControlID::highBandCompressorBypass,
			ControlID::highBandCompressorThreshold,
			ControlID::highBandCompressorAttack,
			ControlID::highBandCompressorRelease,
			ControlID::highBandCompressorRatio
		}
	),
	multiBandEQ(
		stateManager,
		// -- HPF
		ControlID::highpassBypass,
		ControlID::highpassFreq,
		ControlID::highpassSlope,
		// -- LPF
		ControlID::lowpassBypass,
		ControlID::lowpassFreq,
		ControlID::lowpassSlope,
		// -- Band Filters
		{
			ControlID::bandFilter1Bypass,
			ControlID::bandFilter1PeakFreq,
			ControlID::bandFilter1PeakGain,
			ControlID::bandFilter1PeakQ
		},
		{
			ControlID::bandFilter2Bypass,
			ControlID::bandFilter2PeakFreq,
			ControlID::bandFilter2PeakGain,
			ControlID::bandFilter2PeakQ
		},
		{
			ControlID::bandFilter3Bypass,
			ControlID::bandFilter3PeakFreq,
			ControlID::bandFilter3PeakGain,
			ControlID::bandFilter3PeakQ
		}
	),
	imager(
		stateManager,
		ControlID::imagerBypass,
		ControlID::imagerOriginalGain,
		ControlID::imagerAuxiliarGain,
		ControlID::imagerWidth,
		ControlID::imagerCenter,
		ControlID::imagerDelayTime,
		ControlID::imagerCrossoverFreq,
		ControlID::imagerType
	)
{
}

TalkingHeadsPluginAudioProcessor::~TalkingHeadsPluginAudioProcessor()
{
}

//==============================================================================
const juce::String TalkingHeadsPluginAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool TalkingHeadsPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool TalkingHeadsPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool TalkingHeadsPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double TalkingHeadsPluginAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int TalkingHeadsPluginAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int TalkingHeadsPluginAudioProcessor::getCurrentProgram()
{
	return 0;
}

void TalkingHeadsPluginAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String TalkingHeadsPluginAudioProcessor::getProgramName(int index)
{
	return {};
}

void TalkingHeadsPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void TalkingHeadsPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	auto busesLayout = getBusesLayout();
	auto totalNumOutputChannels = busesLayout.getMainOutputChannels();
	// -- Mono Chain
	juce::dsp::ProcessSpec monoSpec{};
	monoSpec.sampleRate = sampleRate;
	monoSpec.maximumBlockSize = samplesPerBlock;
	monoSpec.numChannels = 1;
	// -- Multi channel Chain
	juce::dsp::ProcessSpec multiSpec{};
	multiSpec.sampleRate = sampleRate;
	multiSpec.maximumBlockSize = samplesPerBlock;
	multiSpec.numChannels = totalNumOutputChannels;

	// -- Bypass
	bypass = stateManager->getFloatValue(bypassID);

	// -- Blend
	initBlendMixer(sampleRate, samplesPerBlock);

	// -- pre gain
	preGainGain = stateManager->getFloatValue(preGainID);
	preGain.setRampDurationSeconds(0.01f);
	preGain.setGainDecibels(preGainGain);
	preGain.prepare(monoSpec);

	// -- multi EQ
	multiBandEQ.setSampleRate(monoSpec.sampleRate);
	multiBandEQ.prepare(monoSpec);

	// -- multi Compressor
	multiBandCompressor.prepare(monoSpec);

	// -- Stereo Imager
	imager.prepare(monoSpec);

	// -- phaser
	initPhaser(multiSpec);

	// -- Setup smoothing
	stateManager->resetSmoothedValues(sampleRate);
}

void TalkingHeadsPluginAudioProcessor::initPhaser(const juce::dsp::ProcessSpec& spec)
{
	phaserBypass = stateManager->getBoolValue(phaserBypassID);
	phaserRate = stateManager->getFloatValue(phaserRateID);
	phaserDepth = stateManager->getFloatValue(phaserDepthID);
	phaserCentreFrequency = stateManager->getFloatValue(phaserCentreFrequencyID);
	phaserFeedback = stateManager->getFloatValue(phaserFeedbackID);
	phaserMix = stateManager->getFloatValue(phaserMixID);

	phaser.setRate(phaserRate);
	phaser.setDepth(phaserDepth);
	phaser.setCentreFrequency(phaserCentreFrequency);
	phaser.setFeedback(phaserFeedback);
	phaser.setMix(phaserMix);

	phaser.prepare(spec);
}

void TalkingHeadsPluginAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TalkingHeadsPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
	if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled()
		|| layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
	{
		return false;
	}

	// -- Only mono to stereo supported
	return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

void TalkingHeadsPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
	juce::ScopedNoDenormals noDenormals;

	auto busesLayout = getBusesLayout();
	auto totalNumInputChannels = busesLayout.getMainInputChannels();
	auto totalNumOutputChannels = busesLayout.getMainOutputChannels();
	auto numSamples = buffer.getNumSamples();

	// -- clear output channels not in use
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
	{
		buffer.clear(i, 0, numSamples);
	}

	preProcessBlock();

	if (!juce::approximatelyEqual(bypass, 1.f))
	{
		// -- create the audio blocks and context
		juce::dsp::AudioBlock<float> audioBlock(buffer);
		juce::dsp::ProcessContextReplacing<float> context(audioBlock);

		//const auto& inputBlock = context.getInputBlock();
		auto& outputBlock = context.getOutputBlock();

		// -- prepare dry wet mixer -- dryWetMixer needs an AudioBlock with same number of input and output channels
		blendMixer.setWetLatency(getLatency());
		blendMixer.pushDrySamples(createDryBlock(buffer, totalNumOutputChannels, numSamples));

		// -- Process mono stages
		auto monoBlock = audioBlock.getSingleChannelBlock(0); // -- get the mono block
		juce::dsp::ProcessContextReplacing<float> monoContext(monoBlock);

		preGain.process(monoContext);
		multiBandEQ.process(monoContext);
		multiBandCompressor.process(monoContext);

		// -- Mono to stereo -- context right now has audio only in the mono channel, the imager will transform it to stereo and add width
		imager.process(context);

		// -- Process multi channel stages
		if (!isPhaserBypassed)
		{
			phaser.process(context);
		}

		// -- mix dry wet
		blendMixer.mixWetSamples(outputBlock);
	}

	// TODO: we will need to convert mono to stereo later when adding space (panning etc)
	postProcessBlock();
}

//==============================================================================
bool TalkingHeadsPluginAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TalkingHeadsPluginAudioProcessor::createEditor()
{
	//return new TemplateDSPPluginAudioProcessorEditor (*this, parametersAPVTS, prepareParameterInfoArr);
	return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TalkingHeadsPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto state = stateManager->getAPVTS()->copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void TalkingHeadsPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr && xmlState->hasTagName(stateManager->getAPVTS()->state.getType()))
	{
		stateManager->getAPVTS()->replaceState(juce::ValueTree::fromXml(*xmlState));
	}
}

//==============================================================================
void TalkingHeadsPluginAudioProcessor::reset()
{
	blendMixer.reset();
	preGain.reset();
	multiBandEQ.reset();
	multiBandCompressor.reset();
	imager.reset();
	phaser.reset();
}

//==============================================================================
void TalkingHeadsPluginAudioProcessor::preProcessBlock()
{
	stateManager->syncInBoundVariables();
	postUpdatePluginParameters();
}

void TalkingHeadsPluginAudioProcessor::postProcessBlock()
{
}

void TalkingHeadsPluginAudioProcessor::postUpdatePluginParameters()
{
	float newBypass = stateManager->getCurrentValue(bypassID);
	float newBlend = stateManager->getCurrentValue(blendID);

	if (!juce::approximatelyEqual(newBypass, bypass) || !juce::approximatelyEqual(newBlend, blend))
	{
		bypass = newBypass;
		blend = newBlend;
		blendMixer.setWetMixProportion(std::max(0.f, blend * (1.f - bypass))); // -- if bypassing, all dry. bypass should smooth from 0.f to 1.f when selected.
	}

	float newPreGainGain = stateManager->getCurrentValue(preGainID);
	if (!juce::approximatelyEqual(newPreGainGain, preGainGain))
	{
		preGain.setGainDecibels(stateManager->getCurrentValue(preGainID));
	}

	float newSampleRate = getSampleRate();
	if (!juce::approximatelyEqual(newSampleRate, multiBandEQSampleRate))
	{
		multiBandEQSampleRate = newSampleRate;
		multiBandEQ.setSampleRate(multiBandEQSampleRate);
	}

	postUpdatePhaserParameters();
}

void  TalkingHeadsPluginAudioProcessor::postUpdatePhaserParameters()
{
	float newPhaserBypass = stateManager->getCurrentValue(phaserBypassID);
	bool phaserBypassChanged = !juce::approximatelyEqual(newPhaserBypass, phaserBypass);
	if (phaserBypassChanged)
	{
		phaserBypass = newPhaserBypass;
		isPhaserBypassed = juce::approximatelyEqual(phaserBypass, 1.0f);
	}

	if (isPhaserBypassed)
	{
		return;
	}

	float newPhaserRate = stateManager->getCurrentValue(phaserRateID);
	if (!juce::approximatelyEqual(newPhaserRate, phaserRate))
	{
		phaserRate = newPhaserRate;
		phaser.setRate(phaserRate);
	}

	float newPhaserDepth = stateManager->getCurrentValue(phaserDepthID);
	if (!juce::approximatelyEqual(newPhaserDepth, phaserDepth))
	{
		phaserDepth = newPhaserDepth;
		phaser.setDepth(phaserDepth);
	}

	float newPhaserCentreFrequency = stateManager->getCurrentValue(phaserCentreFrequencyID);
	if (!juce::approximatelyEqual(newPhaserCentreFrequency, phaserCentreFrequency))
	{
		phaserCentreFrequency = newPhaserCentreFrequency;
		phaser.setCentreFrequency(phaserCentreFrequency);
	}

	float newPhaserFeedback = stateManager->getCurrentValue(phaserFeedbackID);
	if (!juce::approximatelyEqual(newPhaserFeedback, phaserFeedback))
	{
		phaserFeedback = newPhaserFeedback;
		phaser.setFeedback(phaserFeedback);
	}

	float newPhaserMix = stateManager->getCurrentValue(phaserMixID);
	if (!juce::approximatelyEqual(newPhaserMix, phaserMix) || phaserBypassChanged)
	{
		phaserMix = newPhaserMix;
		phaser.setMix(juce::jlimit(0.f, 1.f, phaserMix - phaserBypass));
	}
}

float TalkingHeadsPluginAudioProcessor::getLatency()
{
	return 0.f;
}

//==============================================================================
juce::dsp::AudioBlock<float> TalkingHeadsPluginAudioProcessor::createDryBlock(juce::AudioBuffer<float>& buffer, int numChannels, int numSamples)
{
	auto* inputDataPointer = buffer.getReadPointer(0);
	blendMixerBuffer.clear();
	for (int i{ 0 }; i < numChannels; i++)
	{
		blendMixerBuffer.copyFrom(i, 0, inputDataPointer, numSamples);
	}
	return juce::dsp::AudioBlock<float>(blendMixerBuffer);
}

//==============================================================================
void  TalkingHeadsPluginAudioProcessor::initBlendMixer(double sampleRate, int samplesPerBlock)
{
	auto busesLayout = getBusesLayout();
	auto totalNumOutputChannels = busesLayout.getMainOutputChannels();

	juce::dsp::ProcessSpec mixerSpec{};
	mixerSpec.sampleRate = sampleRate;
	mixerSpec.maximumBlockSize = samplesPerBlock;
	mixerSpec.numChannels = totalNumOutputChannels;

	blend = stateManager->getFloatValue(blendID);

	blendMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
	blendMixer.setWetMixProportion(blend);
	blendMixer.prepare(mixerSpec);

	blendMixerBuffer.setSize(totalNumOutputChannels, samplesPerBlock); // -- allocate space
	blendMixerBuffer.clear();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new TalkingHeadsPluginAudioProcessor();
}

