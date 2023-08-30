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
	initPluginParameters(sampleRate);
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
		//gain = juce::Decibels::decibelsToGain(range.convertFrom0to1(gainSmoothedValue.getNextValue()));
		DBG("SAMPLE: " << sample << " GAIN: " << gain);
	}


	//for (int channel = 0; channel < totalNumInputChannels; ++channel)
	//{
	//	auto* channelData = buffer.getWritePointer(channel);

	//	for (int sample = 0; sample < numSamples; ++sample) {

	//		float x_n = channelData[sample];

	//		channelData[sample] = invertPhase * gain * x_n;
	//	}

	//}

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

	// TODO: check that num ids are valid XML names
	for (int i{ 0 }; i < ControlID::countParams; ++i)
	{
		PrepareParameterInfo info = prepareParameterInfoArr[i];
		switch (info.parameterType)
		{
		case ParameterType::Bool:
			layout.add(std::make_unique<juce::AudioParameterBool>(
				info.parameterID,
				info.name,
				info.boolDefaultValue,
				juce::AudioParameterBoolAttributes().withLabel(info.label)
			));
			break;
		case ParameterType::Choice:
			layout.add(std::make_unique<juce::AudioParameterChoice>(
				info.parameterID,
				info.name,
				info.choices,
				info.defaultItemIndex,
				juce::AudioParameterChoiceAttributes().withLabel(info.label)
			));
			break;
		case ParameterType::Float:
			layout.add(std::make_unique<juce::AudioParameterFloat>(
				info.parameterID,
				info.name,
				info.floatRange,
				info.floatDefaultValue,
				juce::AudioParameterFloatAttributes().withLabel(info.label)
			));
			break;
		case ParameterType::Int:
			layout.add(std::make_unique<juce::AudioParameterInt>(
				info.parameterID,
				info.name,
				info.minValue,
				info.maxValue,
				info.intDefaultValue,
				juce::AudioParameterIntAttributes().withLabel(info.label)
			));
			break;
		}
	}

	return layout;
}

//==============================================================================
void TemplateDSPPluginAudioProcessor::initPluginParameters(double sampleRate)
{
	// -- initialize the parameters
	for (int i{ 0 }; i < ControlID::countParams; ++i)
	{
		if (prepareParameterInfoArr[i].smoothingType == SmoothingType::NoSmoothing)
		{
			pluginProcessorParameters[i] = ParameterObject(
				parametersAPVTS.getParameter(prepareParameterInfoArr[i].getParamID()),
				prepareParameterInfoArr[i].parameterType
			);
		}
		else {
			pluginProcessorParameters[i] = ParameterObject(
				parametersAPVTS.getParameter(prepareParameterInfoArr[i].getParamID()),
				prepareParameterInfoArr[i].smoothingType,
				sampleRate
			);
		}
	}
}

void TemplateDSPPluginAudioProcessor::initPluginMemberVariables(double sampleRate) {
	// -- initialize the member variables
	gain = juce::Decibels::decibelsToGain(pluginProcessorParameters[ControlID::gain].getFloatValue());
	//gain = juce::Decibels::decibelsToGain(pluginProcessorParameters[ControlID::gain].getSmoothedCurrentValue());

	// -- TMP TO FIX SMOOTHING
	//range = parametersAPVTS.getParameter(prepareParameterInfoArr[ControlID::gain].getParamID())->getNormalisableRange();
	pluginProcessorParameters[ControlID::gain].reset(sampleRate, 0.05f);
	pluginProcessorParameters[ControlID::gain].setCurrentAndTargetValue(gain);
	//gainSmoothedValue.reset(sampleRate, 0.05f);
	//gainSmoothedValue.setCurrentAndTargetValue(range.convertTo0to1(gain));

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
	pluginProcessorParameters[ControlID::gain].setTargetValue(pluginProcessorParameters[ControlID::gain].getFloatValue());
	//gainSmoothedValue.setTargetValue(range.convertTo0to1(pluginProcessorParameters[ControlID::gain].getFloatValue()));
}

bool TemplateDSPPluginAudioProcessor::postUpdatePluginParameter(ControlID controlID)
{
	// Cooking and transfer raw parameters to member variables
	switch (controlID)
	{
	case ControlID::gain:
	{
		gain = juce::Decibels::decibelsToGain(pluginProcessorParameters[ControlID::gain].getSmoothedValue());
		//gain = juce::Decibels::decibelsToGain(range.convertFrom0to1(gainSmoothedValue.getNextValue()));
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

