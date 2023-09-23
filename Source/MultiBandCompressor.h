/*
  ==============================================================================

	MultiBandCompressor.h
	Created: 22 Sep 2023 5:01:45pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "ParameterDefinition.h"
#include "ParameterObject.h"
#include "CompressorBand.h"

//==============================================================================
class MultiBandCompressor : public juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	MultiBandCompressor(
		std::array<ParameterDefinition, ControlID::countParams>(&parameterDefinitions),
		std::array<ParameterObject, ControlID::countParams>(&pluginProcessorParameters)
	);
	~MultiBandCompressor();

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	float getLatency();

private:
	// TODO: abstract the class from the processor stage of the plugin so that it can be used separately
	//==============================================================================
	// --- Object parameters management and information
	const std::set<ControlID> multiBandCompressorIDs = {
		// -- Low Band Compressor
		ControlID::lowBandCompressorBypass,
		// -- Mid Band Compressor
		// -- High Band Compressor
	};

	std::array<ParameterDefinition, ControlID::countParams>(*parameterDefinitions) { nullptr };
	std::array<ParameterObject, ControlID::countParams>(*pluginProcessorParameters) { nullptr };

	//==============================================================================
	// --- Object member variables

	// -- Band filters
	float lowMidCrossoverFreq{ 0.f };
	// -- Low Band Compressor
	float lowBandCompressorBypass{ 0.f };

	//==============================================================================
	// -- Band filters
	using Filter = juce::dsp::LinkwitzRileyFilter<float>;

	//      fc0     fc1
	Filter  LP, HP;

	std::array<juce::AudioBuffer<float>, 2> filterBuffers;


	// -- Processor Chain
	enum chainIndex
	{
		lowBandCompressorChainIndex,
		//midBandCompressorChainIndex,
		//highBandCompressorChainIndex
	};

	juce::dsp::ProcessorChain<CompressorBand> processorChain;

	//==============================================================================
	void preProcess();

	void syncInBoundVariables();
	void postUpdatePluginParameters();
};