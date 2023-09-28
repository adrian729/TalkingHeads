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

		// -- Process stages
		juce::dsp::ProcessContextReplacing<float> monoContext(monoBlock);

		// -- Pregain
		preGainProcessor.process(monoContext);

		// -- First stage -- EQ
		firstStageProcessor.process(monoContext);
		// -- Second stage -- Compressor
		secondStageProcessor.process(monoContext);

		// -- Mono to stereo -- copy the mono channel to all output channels
		for (int i{ 1 }; i < totalNumOutputChannels; i++)
		{
			audioBlock.getSingleChannelBlock(i).copyFrom(monoBlock);
		}

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
	preGainProcessor.reset();
	firstStageProcessor.reset();
}

//==============================================================================
void TalkingHeadsPluginAudioProcessor::preProcessBlock()
{
	syncInBoundVariables();

	firstStageProcessor.setSampleRate(getSampleRate());
}

void TalkingHeadsPluginAudioProcessor::postProcessBlock()
{

}

void TalkingHeadsPluginAudioProcessor::syncInBoundVariables() {
	for (auto& id : generalControlIDs)
	{
		pluginProcessorParameters[id].updateInBoundVariable();
		postUpdatePluginParameter(id);
	}

	// -- if bypassing, add bypass to dry/wet mixer
	blendMixer.setWetMixProportion(std::max(0.f, blend - bypass)); // -- if bypassing, all dry. bypass should smooth from 0.f to 1.f when selected.
}

bool TalkingHeadsPluginAudioProcessor::postUpdatePluginParameter(ControlID controlID)
{
	// Cooking and transfer raw parameters to member variables
	switch (controlID)
	{
	case ControlID::bypass:
	{
		bypass = pluginProcessorParameters[controlID].getNextValue();
		break;
	}
	case ControlID::blend:
	{
		blend = pluginProcessorParameters[controlID].getNextValue();
		break;
	}
	case ControlID::preGain:
	{
		preGainProcessor.setGainDecibels(pluginProcessorParameters[controlID].getFloatValue());
		break;
	}
	default:
		return false;
	}

	return true;
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

	juce::dsp::ProcessSpec monoSpec;
	monoSpec.sampleRate = sampleRate;
	monoSpec.maximumBlockSize = samplesPerBlock;
	monoSpec.numChannels = 1;

	// -- initialize the member variables
	// -- Bypass
	bypass = pluginProcessorParameters[ControlID::bypass].getFloatValue();

	// -- Blend
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

	// -- Gain
	preGainProcessor.setRampDurationSeconds(0.01f);
	preGainProcessor.setGainDecibels(pluginProcessorParameters[ControlID::preGain].getFloatValue());
	preGainProcessor.prepare(monoSpec);

	// -- initialize first stage processor
	firstStageProcessor.prepare(monoSpec);

	// -- initialize second stage processor
	secondStageProcessor.setupMultiBandCompressor(
		parameterDefinitions,
		pluginProcessorParameters,
		ControlID::lowMidCrossoverFreq,
		ControlID::midHighCrossoverFreq
	);
	// -- Low band filter -- lowpass1 -> allpass = lowpass band (----\) -- we add an allpass to avoid phase issues
	secondStageProcessor.getLowBandCompressor()->setupCompressorBand(
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
	secondStageProcessor.getMidBandCompressor()->setupCompressorBand(
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
	secondStageProcessor.getHighBandCompressor()->setupCompressorBand(
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

	secondStageProcessor.prepare(monoSpec);

	// -- Setup smoothing
	pluginProcessorParameters[ControlID::bypass].initSmoothing(sampleRate, 0.01f);
	pluginProcessorParameters[ControlID::blend].initSmoothing(sampleRate);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new TalkingHeadsPluginAudioProcessor();
}

