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
	initPluginMemberVariables(sampleRate);
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

	// Only mono/stereo are supported
	return layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo();
}
#endif

void TalkingHeadsPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();
	auto numSamples = buffer.getNumSamples();

	// -- clear output channels not in use
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
	{
		buffer.clear(i, 0, numSamples);
	}

	preProcessBlock();

	// -- TMP CHECKING WITH PROCESS BY FRAME
	for (int sample = 0; sample < numSamples; ++sample) {

		// -- frame: data of all channels for one sample
		for (int channel = 0; channel < totalNumInputChannels; ++channel)
		{
			if (juce::approximatelyEqual(bypass, 1.f))
			{
				break;
			}

			auto* channelData = buffer.getWritePointer(channel);

			float x_n = channelData[sample]; // -- input sample

			float fx_n = gain * x_n; // -- fx process

			float wet = fx_n;
			float dry = x_n;
			float dryWetMix = blendValues(wet, dry, blend);

			channelData[sample] = blendValues(x_n, dryWetMix, bypass);
		}

		// -- update values that use smoothing
		bypass = pluginProcessorParameters[ControlID::bypass].getNextValue();
		gain = juce::Decibels::decibelsToGain(pluginProcessorParameters[ControlID::gain].getNextValue());
	}

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
void TalkingHeadsPluginAudioProcessor::preProcessBlock()
{
	syncInBoundVariables();
}

void TalkingHeadsPluginAudioProcessor::postProcessBlock()
{

}

void TalkingHeadsPluginAudioProcessor::syncInBoundVariables() {
	for (int i = 0; i < ControlID::countParams; ++i)
	{
		pluginProcessorParameters[i].updateInBoundVariable();
		postUpdatePluginParameter(intToEnum(i, ControlID));
	}
}

bool TalkingHeadsPluginAudioProcessor::postUpdatePluginParameter(ControlID controlID)
{
	// Cooking and transfer raw parameters to member variables
	switch (controlID)
	{
	case ControlID::bypass:
	{
		bypass = pluginProcessorParameters[ControlID::bypass].getNextValue();
		break;
	}
	case ControlID::blend:
	{
		blend = pluginProcessorParameters[ControlID::blend].getNextValue();
		break;
	}
	case ControlID::gain:
	{
		gain = juce::Decibels::decibelsToGain(pluginProcessorParameters[ControlID::gain].getNextValue());
		break;
	}
	default:
		return false;
	}

	return true;
}

//==============================================================================
float TalkingHeadsPluginAudioProcessor::blendValues(float wet, float dry, float blend)
{
	return blend * wet + (1.f - blend) * dry;
}

//==============================================================================
const std::array<ParameterDefinition, ControlID::countParams> TalkingHeadsPluginAudioProcessor::createParameterDefinitions()
{
	std::array<ParameterDefinition, ControlID::countParams> tmp_parameterDefinitions;

	// -- using a float to be able to smooth the bypass transition
	tmp_parameterDefinitions[ControlID::bypass] = ParameterDefinition(
		ControlID::bypass,
		1,
		"Bypass",
		juce::NormalisableRange<float>(0.f, 1.f, 1.f),
		0.f,
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::blend] = ParameterDefinition(
		ControlID::blend,
		1,
		"Blend",
		juce::NormalisableRange<float>(0.f, 1.f, 0.01f),
		1.f,
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::gain] = ParameterDefinition(
		ControlID::gain,
		1,
		"Gain",
		juce::NormalisableRange<float>(-24.f, 24.f, 0.01f),
		0.f,
		SmoothingType::Linear,
		"dB"
	);

	return tmp_parameterDefinitions;
}

juce::AudioProcessorValueTreeState::ParameterLayout TalkingHeadsPluginAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	// TODO: check that num ids are valid XML names -------------------------------
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
		if (parameterDefinitions[i].getSmoothingType() == SmoothingType::NoSmoothing)
		{
			pluginProcessorParameters[i] = ParameterObject(
				parametersAPVTS.getParameter(parameterDefinitions[i].getParamID()),
				parameterDefinitions[i].getParameterType()
			);
		}
		else {
			pluginProcessorParameters[i] = ParameterObject(
				parametersAPVTS.getParameter(parameterDefinitions[i].getParamID()),
				parameterDefinitions[i].getSmoothingType()
			);
		}
	}
}

void TalkingHeadsPluginAudioProcessor::initPluginMemberVariables(double sampleRate) {
	// -- initialize the member variables
	bypass = pluginProcessorParameters[ControlID::bypass].getFloatValue();
	blend = pluginProcessorParameters[ControlID::blend].getFloatValue();
	gain = juce::Decibels::decibelsToGain(pluginProcessorParameters[ControlID::gain].getFloatValue());

	// -- Setup smoothing
	pluginProcessorParameters[ControlID::bypass].initSmoothing(sampleRate);
	pluginProcessorParameters[ControlID::blend].initSmoothing(sampleRate);
	pluginProcessorParameters[ControlID::gain].initSmoothing(sampleRate);

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new TalkingHeadsPluginAudioProcessor();
}

