/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "ParameterObject.h"
#include "ParameterDefinition.h"

//==============================================================================
// --- PARAMETER IDs
enum ControlID {
	// -- stage 1: general
	bypass,
	blend,
	gain,
	// -- stage 2: EQ filter
	//highPassFreq,
	//lowCutSlope,
	//==============================================================================
	countParams // value to keep track of the total number of parameters
};
// TODO: each param ID should be UNIQUE and should NEVER CHANGE once released, so better to add some mapping from ControlID to param ID and use the param ID in the apvts?

//==============================================================================
//template <typename Type>
//class Distortion
//{
//public:
//	//==============================================================================
//	Distortion()
//	{
//		auto& waveshaper = processorChain.template get<waveshaperIndex>();
//		waveshaper.functionToUse = [](Type x)
//		{
//			return std::tanh(x);
//		};
//
//		auto& preGain = processorChain.template get<preGainIndex>();
//		preGain.setGainDecibels(30.0f);
//
//		auto& postGain = processorChain.template get<postGainIndex>();
//		postGain.setGainDecibels(-20.0f);
//	}
//
//	//==============================================================================
//	void prepare(const juce::dsp::ProcessSpec& spec)
//	{
//		auto& filter = processorChain.template get<filterIndex>();
//		filter.state = FilterCoefs::makeFirstOrderHighPass(spec.sampleRate, 1000.0f);
//
//		processorChain.prepare(spec);
//	}
//
//	//==============================================================================
//	template <typename ProcessContext>
//	void process(const ProcessContext& context) noexcept
//	{
//		processorChain.process(context);
//	}
//
//	//==============================================================================
//	void reset() noexcept
//	{
//		processorChain.reset();
//	}
//
//private:
//	//==============================================================================
//	enum
//	{
//		filterIndex,
//		preGainIndex,
//		waveshaperIndex,
//		postGainIndex
//	};
//
//	using Filter = juce::dsp::StateVariableFilter::Filter<Type>;
//	using FilterCoefs = juce::dsp::StateVariableFilter::Coefficients<Type>;
//
//	juce::dsp::ProcessorChain<
//		juce::dsp::ProcessorDuplicator<Filter, FilterCoefs>,
//		juce::dsp::Gain<Type>,
//		juce::dsp::WaveShaper<Type>,
//		juce::dsp::Gain<Type>
//	> processorChain;
//};


//==============================================================================
/**
*/
class TalkingHeadsPluginAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	, public juce::AudioProcessorARAExtension
#endif
{
public:
	//==============================================================================
	TalkingHeadsPluginAudioProcessor();
	~TalkingHeadsPluginAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	//==============================================================================
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

private:
	//==============================================================================
	// --- Object parameters management and information

	// --- Parameters definitions - make sure each item in the array matches the order of the enum
	const std::array<ParameterDefinition, ControlID::countParams> parameterDefinitions = createParameterDefinitions();

	// --- Parameters state APVTS
	const juce::String PARAMETERS_APVTS_ID = "ParametersAPVTS";
	juce::AudioProcessorValueTreeState parametersAPVTS;

	// --- Object parameters and inBound variables
	ParameterObject pluginProcessorParameters[ControlID::countParams];

	//==============================================================================
	// --- Object member variables

	// -- General process
	float bypass; // -- using a float to smooth the bypass transition
	float blend;
	float gain;

	//==============================================================================
	const std::array<ParameterDefinition, ControlID::countParams> createParameterDefinitions();
	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	void initPluginParameters();
	void initPluginMemberVariables(double sampleRate);

	//==============================================================================
	void preProcessBlock();
	void postProcessBlock();

	void syncInBoundVariables();
	bool postUpdatePluginParameter(ControlID parameterID);

	//==============================================================================
	float blendValues(float dry, float wet, float blend);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TalkingHeadsPluginAudioProcessor)
};
