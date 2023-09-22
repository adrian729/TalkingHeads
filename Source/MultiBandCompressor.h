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
	const std::set<ControlID> firstStageControlIDs = {
		// -- Low Band Compressor
		ControlID::lowBandCompressorBypass,
		ControlID::lowBandCompressorThreshold,
		ControlID::lowBandCompressorAttack,
		ControlID::lowBandCompressorRelease,
		ControlID::lowBandCompressorRatio
		// -- Mid Band Compressor
		// -- High Band Compressor
	};

	std::array<ParameterDefinition, ControlID::countParams>(*parameterDefinitions) { nullptr };
	std::array<ParameterObject, ControlID::countParams>(*pluginProcessorParameters) { nullptr };

	//==============================================================================
	// --- Object member variables

	// -- Low Band Compressor
	//float lowBandCompressorThreshold{ 0.f };
	//float lowBandCompressorAttack{ 0.f };
	//float lowBandCompressorRelease{ 0.f };
	//float lowBandCompressorRatio{ 0.f };

	//==============================================================================
	// -- Processor Chain
	enum chainIndex
	{
		lowBandCompressorChainIndex,
		//midBandCompressorChainIndex,
		//highBandCompressorChainIndex
	};

	using Compressor = juce::dsp::Compressor<float>;

	juce::dsp::ProcessorChain<Compressor> processorChain;

	//==============================================================================
	void preProcess();
	void postProcess();

	void syncInBoundVariables();
	bool postUpdatePluginParameter(ControlID controlID);
};