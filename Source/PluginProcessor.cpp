/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TalkingHeadsPluginAudioProcessor::TalkingHeadsPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
		.withInput("Input", juce::AudioChannelSet::mono(), true)
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
	),
#endif
	parametersAPVTS(*this, nullptr, juce::Identifier(PARAMETERS_APVTS_ID), createParameterLayout())
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
	// -- create the parameters and inBound variables
	initPluginParameters();
	// -- init the member variables
	initPluginMemberVariables(sampleRate, samplesPerBlock);
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
		auto monoBlock = audioBlock.getSingleChannelBlock(0); // -- get the mono block

		// TODO: change so that we work with a mono block/context in the first stages, 
		// and then check how we convert to stereo-stereo after we are done with that
		// OR separate in different blocks/context/buffers for each voice before we mix them and apply last stages

		juce::dsp::ProcessContextReplacing<float> context(audioBlock);

		//const auto& inputBlock = context.getInputBlock();
		auto& outputBlock = context.getOutputBlock();

		// -- prepare dry wet mixer -- dryWetMixer needs an AudioBlock with same number of input and output channels
		blendMixer.setWetLatency(getLatency());
		blendMixer.pushDrySamples(createDryBlock(buffer, totalNumOutputChannels, numSamples));

		// -- Process mono stages
		juce::dsp::ProcessContextReplacing<float> monoContext(monoBlock);

		monoChain.process(monoContext);

		// -- Mono to stereo -- copy the mono channel to all output channels
		for (int i{ 1 }; i < totalNumOutputChannels; i++)
		{
			outputBlock.getSingleChannelBlock(i).copyFrom(monoBlock);
		}

		// -- Process multi channel stages
		multiChannelChain.process(context);

		// -- mix dry wet
		blendMixer.mixWetSamples(outputBlock);
	}

	// TODO: we will need to convert mono to stereo later when adding space (panning etc)
	postProcessBlock();
}

float TalkingHeadsPluginAudioProcessor::getIndexInterpolationFactor(int originSize, int newSize)
{
	// originIndex = newIndex * factor
	return (float)(originSize - 1) / (float)(newSize - 1);
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
	auto state = parametersAPVTS.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void TalkingHeadsPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr && xmlState->hasTagName(parametersAPVTS.state.getType()))
	{
		parametersAPVTS.replaceState(juce::ValueTree::fromXml(*xmlState));
	}
}

//==============================================================================
void TalkingHeadsPluginAudioProcessor::reset()
{
	blendMixer.reset();

	monoChain.reset();
}

//==============================================================================
void TalkingHeadsPluginAudioProcessor::preProcessBlock()
{
	syncInBoundVariables();
	postUpdatePluginParameters();
}

void TalkingHeadsPluginAudioProcessor::postProcessBlock()
{
}

void TalkingHeadsPluginAudioProcessor::syncInBoundVariables() {
	// -- In the main processor we update all variables
	for (auto& param : pluginProcessorParameters)
	{
		param.updateInBoundVariable();
	}
}

void TalkingHeadsPluginAudioProcessor::postUpdatePluginParameters()
{
	bypass = pluginProcessorParameters[bypassID].getCurrentValue();
	blend = pluginProcessorParameters[blendID].getCurrentValue();

	blendMixer.setWetMixProportion(std::max(0.f, blend - bypass)); // -- if bypassing, all dry. bypass should smooth from 0.f to 1.f when selected.

	auto& preGain = monoChain.template get<preGainIndex>();
	preGain.setGainDecibels(pluginProcessorParameters[preGainID].getCurrentValue());

	phaserBypass = pluginProcessorParameters[phaserBypassID].getBoolValue();
	multiChannelChain.template setBypassed<phaserIndex>(phaserBypass);

	if (!phaserBypass)
	{
		auto& phaser = multiChannelChain.template get<phaserIndex>();
		phaser.setRate(pluginProcessorParameters[phaserRateID].getCurrentValue());
		phaser.setDepth(pluginProcessorParameters[phaserDepthID].getCurrentValue());
		phaser.setCentreFrequency(pluginProcessorParameters[phaserCentreFrequencyID].getCurrentValue());
		phaser.setFeedback(pluginProcessorParameters[phaserFeedbackID].getCurrentValue());
		phaser.setMix(pluginProcessorParameters[phaserMixID].getCurrentValue());
	}


	auto& multiBandEQ = monoChain.template get<multiEQIndex>();
	multiBandEQ.setSampleRate(getSampleRate());
}

float TalkingHeadsPluginAudioProcessor::getLatency()
{
	return 0.f;
}

//==============================================================================
float TalkingHeadsPluginAudioProcessor::blendValues(float wet, float dry, float blend)
{
	return blend * wet + (1.f - blend) * dry;
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
juce::AudioProcessorValueTreeState::ParameterLayout TalkingHeadsPluginAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	for (int i{ 0 }; i < ControlID::countParams; ++i)
	{
		layout.add(parameterDefinitions[i].createParameter());
	}

	return layout;
}

void TalkingHeadsPluginAudioProcessor::initPluginParameters()
{
	// -- initialize the parameters
	for (int i{ 0 }; i < ControlID::countParams; ++i)
	{
		pluginProcessorParameters[i] = ParameterObject(
			parametersAPVTS.getParameter(parameterDefinitions[i].getParamID()),
			parameterDefinitions[i].getParameterType(),
			parameterDefinitions[i].getSmoothingType()
		);
	}
}

void TalkingHeadsPluginAudioProcessor::initPluginMemberVariables(double sampleRate, int samplesPerBlock)
{
	auto busesLayout = getBusesLayout();
	auto totalNumOutputChannels = busesLayout.getMainOutputChannels();

	// -- Bypass
	bypass = pluginProcessorParameters[ControlID::bypass].getFloatValue();

	// -- Blend
	initBlendMixer(sampleRate, samplesPerBlock);

	// -- pre gain
	auto& preGain = monoChain.template get<preGainIndex>();
	preGain.setRampDurationSeconds(0.01f);
	preGain.setGainDecibels(pluginProcessorParameters[ControlID::preGain].getFloatValue());
	// -- multi EQ
	initMonoMultiEQ();
	// -- multi Compressor
	initMonoMultiCompressor();
	// -- phaser
	phaserBypass = pluginProcessorParameters[phaserBypassID].getBoolValue();
	multiChannelChain.template setBypassed<phaserIndex>(phaserBypass);

	auto& phaser = multiChannelChain.template get<phaserIndex>();
	phaser.setRate(pluginProcessorParameters[ControlID::phaserRate].getFloatValue());
	phaser.setDepth(pluginProcessorParameters[ControlID::phaserDepth].getFloatValue());
	phaser.setCentreFrequency(pluginProcessorParameters[ControlID::phaserCentreFrequency].getFloatValue());
	phaser.setFeedback(pluginProcessorParameters[ControlID::phaserFeedback].getFloatValue());
	phaser.setMix(pluginProcessorParameters[ControlID::phaserMix].getFloatValue());

	// -- Mono Chain
	juce::dsp::ProcessSpec monoSpec;
	monoSpec.sampleRate = sampleRate;
	monoSpec.maximumBlockSize = samplesPerBlock;
	monoSpec.numChannels = 1;
	monoChain.prepare(monoSpec);

	// -- Multi channel Chain
	juce::dsp::ProcessSpec multiSpec;
	multiSpec.sampleRate = sampleRate;
	multiSpec.maximumBlockSize = samplesPerBlock;
	multiSpec.numChannels = totalNumOutputChannels;
	multiChannelChain.prepare(multiSpec);

	// -- Setup smoothing
	for (int i{ 0 }; i < ControlID::countParams; ++i)
	{
		pluginProcessorParameters[i].initSmoothing(sampleRate, parameterDefinitions[i].getRampLengthInSeconds());
	}
}

void  TalkingHeadsPluginAudioProcessor::initBlendMixer(double sampleRate, int samplesPerBlock)
{
	auto busesLayout = getBusesLayout();
	auto totalNumOutputChannels = busesLayout.getMainOutputChannels();

	juce::dsp::ProcessSpec mixerSpec;
	mixerSpec.sampleRate = sampleRate;
	mixerSpec.maximumBlockSize = samplesPerBlock;
	mixerSpec.numChannels = totalNumOutputChannels;

	blend = pluginProcessorParameters[ControlID::blend].getFloatValue();

	blendMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
	blendMixer.setWetMixProportion(blend);
	blendMixer.prepare(mixerSpec);

	blendMixerBuffer.setSize(totalNumOutputChannels, samplesPerBlock); // -- allocate space
	blendMixerBuffer.clear();
}

void TalkingHeadsPluginAudioProcessor::initMonoMultiEQ()
{
	auto& multiBandEQ = monoChain.template get<multiEQIndex>();
	multiBandEQ.setupMultiBandEQ(
		parameterDefinitions,
		pluginProcessorParameters,
		// -- HPF
		ControlID::highpassBypass,
		ControlID::highpassFreq,
		ControlID::highpassSlope,
		// -- LPF
		ControlID::lowpassBypass,
		ControlID::lowpassFreq,
		ControlID::lowpassSlope,
		// -- Band Filters
		ControlID::bandFilter1Bypass,
		ControlID::bandFilter2Bypass,
		ControlID::bandFilter3Bypass
	);
	// -- band filter 1
	multiBandEQ.getFirstBandFilter()->setupEQBand(
		parameterDefinitions,
		pluginProcessorParameters,
		ControlID::bandFilter1Bypass,
		ControlID::bandFilter1PeakFreq,
		ControlID::bandFilter1PeakGain,
		ControlID::bandFilter1PeakQ
	);
	// -- band filter 2
	multiBandEQ.getSecondBandFilter()->setupEQBand(
		parameterDefinitions,
		pluginProcessorParameters,
		ControlID::bandFilter2Bypass,
		ControlID::bandFilter2PeakFreq,
		ControlID::bandFilter2PeakGain,
		ControlID::bandFilter2PeakQ
	);
	// -- band filter 3
	multiBandEQ.getThirdBandFilter()->setupEQBand(
		parameterDefinitions,
		pluginProcessorParameters,
		ControlID::bandFilter3Bypass,
		ControlID::bandFilter3PeakFreq,
		ControlID::bandFilter3PeakGain,
		ControlID::bandFilter3PeakQ
	);
}

void TalkingHeadsPluginAudioProcessor::initMonoMultiCompressor()
{
	auto& multiBandCompressor = monoChain.template get<multiCompIndex>();
	// -- Low band filter -- lowpass1 -> allpass = lowpass band (----\) -- we add an allpass to avoid phase issues
	multiBandCompressor.getLowBandCompressor()->setupCompressorBand(
		parameterDefinitions,
		pluginProcessorParameters,
		// -- Compressor
		ControlID::lowBandCompressorBypass,
		ControlID::lowBandCompressorThreshold,
		ControlID::lowBandCompressorAttack,
		ControlID::lowBandCompressorRelease,
		ControlID::lowBandCompressorRatio,
		// -- Filters
		ControlID::lowMidCrossoverFreq,
		ControlID::countParams,
		juce::dsp::LinkwitzRileyFilterType::lowpass,
		juce::dsp::LinkwitzRileyFilterType::allpass
	);
	// -- mid band filter -- highpass1 -> lowpass2 = mid band (/---\)
	multiBandCompressor.getMidBandCompressor()->setupCompressorBand(
		parameterDefinitions,
		pluginProcessorParameters,
		// -- Compressor
		ControlID::midBandCompressorBypass,
		ControlID::midBandCompressorThreshold,
		ControlID::midBandCompressorAttack,
		ControlID::midBandCompressorRelease,
		ControlID::midBandCompressorRatio,
		// -- Filters
		ControlID::lowMidCrossoverFreq,
		ControlID::midHighCrossoverFreq,
		juce::dsp::LinkwitzRileyFilterType::lowpass,
		juce::dsp::LinkwitzRileyFilterType::highpass
	);
	// -- high band filter -- highpass1 -> highpass2 = highpass band (/----)
	multiBandCompressor.getHighBandCompressor()->setupCompressorBand(
		parameterDefinitions,
		pluginProcessorParameters,
		// -- Compressor
		ControlID::highBandCompressorBypass,
		ControlID::highBandCompressorThreshold,
		ControlID::highBandCompressorAttack,
		ControlID::highBandCompressorRelease,
		ControlID::highBandCompressorRatio,
		// -- Filters
		ControlID::countParams,
		ControlID::midHighCrossoverFreq,
		juce::dsp::LinkwitzRileyFilterType::allpass,
		juce::dsp::LinkwitzRileyFilterType::highpass
	);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new TalkingHeadsPluginAudioProcessor();
}

