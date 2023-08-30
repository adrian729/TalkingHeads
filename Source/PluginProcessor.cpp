/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TemplateDSPPluginAudioProcessor::TemplateDSPPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	),
#endif
	parametersAPVTS(*this, nullptr, juce::Identifier(PARAMETERS_APVTS_ID), createParameterLayout())
{
}

TemplateDSPPluginAudioProcessor::~TemplateDSPPluginAudioProcessor()
{
}

//==============================================================================
const juce::String TemplateDSPPluginAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool TemplateDSPPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool TemplateDSPPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool TemplateDSPPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double TemplateDSPPluginAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int TemplateDSPPluginAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int TemplateDSPPluginAudioProcessor::getCurrentProgram()
{
	return 0;
}

void TemplateDSPPluginAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String TemplateDSPPluginAudioProcessor::getProgramName(int index)
{
	return {};
}

void TemplateDSPPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void TemplateDSPPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	// -- create the parameters and inBound variables
	initPluginParameters();
	// -- init the member variables
	initPluginMemberVariables(sampleRate);
}

void TemplateDSPPluginAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TemplateDSPPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
	if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled()
		|| layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
	{
		return false;
	}

	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
	{
		return false;
	}

	return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}
#endif

void TemplateDSPPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
			auto* channelData = buffer.getWritePointer(channel);

			float x_n = channelData[sample];

			channelData[sample] = invertPhase * gain * x_n;
		}

		// -- update values that use smoothing
		gain = juce::Decibels::decibelsToGain(pluginProcessorParameters[ControlID::gain].getSmoothedValue());
	}

	postProcessBlock();
}

//==============================================================================
bool TemplateDSPPluginAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TemplateDSPPluginAudioProcessor::createEditor()
{
	//return new TemplateDSPPluginAudioProcessorEditor (*this, parametersAPVTS, prepareParameterInfoArr);
	return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TemplateDSPPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto state = parametersAPVTS.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void TemplateDSPPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr && xmlState->hasTagName(parametersAPVTS.state.getType()))
	{
		parametersAPVTS.replaceState(juce::ValueTree::fromXml(*xmlState));
	}
}

juce::AudioProcessorValueTreeState::ParameterLayout TemplateDSPPluginAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	// TODO: check that num ids are valid XML names -------------------------------
	for (int i{ 0 }; i < ControlID::countParams; ++i)
	{
		layout.add(parameterDefinitions[i].createParameter());
	}

	return layout;
}

//==============================================================================
void TemplateDSPPluginAudioProcessor::initPluginParameters()
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

void TemplateDSPPluginAudioProcessor::initPluginMemberVariables(double sampleRate) {
	// -- initialize the member variables
	gain = juce::Decibels::decibelsToGain(pluginProcessorParameters[ControlID::gain].getFloatValue());

	// -- Setup smoothing
	pluginProcessorParameters[ControlID::gain].initSmoothing(sampleRate);

	invertPhase = pluginProcessorParameters[ControlID::invertPhase].getBoolValue() ? -1 : 1;
}

//==============================================================================
void TemplateDSPPluginAudioProcessor::preProcessBlock()
{
	syncInBoundVariables();
}

void TemplateDSPPluginAudioProcessor::postProcessBlock()
{

}

void TemplateDSPPluginAudioProcessor::syncInBoundVariables() {
	for (int i = 0; i < ControlID::countParams; ++i)
	{
		pluginProcessorParameters[i].updateInBoundVariable();
		postUpdatePluginParameter(intToEnum(i, ControlID));
	}
}

bool TemplateDSPPluginAudioProcessor::postUpdatePluginParameter(ControlID controlID)
{
	// Cooking and transfer raw parameters to member variables
	switch (controlID)
	{
	case ControlID::gain:
	{
		gain = juce::Decibels::decibelsToGain(pluginProcessorParameters[ControlID::gain].getSmoothedValue());
		break;
	}
	case ControlID::invertPhase:
	{
		invertPhase = pluginProcessorParameters[ControlID::invertPhase].getBoolValue() ? -1 : 1;
		break;
	}
	default:
		return false;
	}

	return true;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new TemplateDSPPluginAudioProcessor();
}

