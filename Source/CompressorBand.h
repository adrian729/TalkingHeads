/*
  ==============================================================================

	CompressorBand.h
	Created: 23 Sep 2023 1:20:20pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "ParameterDefinition.h"
#include "ParameterObject.h"

class CompressorBand : public  juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	CompressorBand();
	~CompressorBand();

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	float getLatency();

	//==============================================================================
	void setupCompressorBand(
		std::array<ParameterDefinition, ControlID::countParams>& parameterDefinitions,
		std::array<ParameterObject, ControlID::countParams>& pluginProcessorParameters,
		ControlID bypassID,
		ControlID thresholdID,
		ControlID attackID,
		ControlID releaseID,
		ControlID ratioID
	);

private:
	//==============================================================================
	// --- Object parameters management and information
	std::set<ControlID> compressorBandControlIDs;

	std::array<ParameterDefinition, ControlID::countParams>(*parameterDefinitions) { nullptr };
	std::array<ParameterObject, ControlID::countParams>(*pluginProcessorParameters) { nullptr };

	//==============================================================================
	// --- Object member variables
	ControlID bypassID{ ControlID::countParams };
	ControlID thresholdID{ ControlID::countParams };
	ControlID attackID{ ControlID::countParams };
	ControlID releaseID{ ControlID::countParams };
	ControlID ratioID{ ControlID::countParams };

	float bypass{ 0.f };

	juce::dsp::Compressor<float> compressor;

	//==============================================================================
	void preProcess();

	void syncInBoundVariables();
	void postUpdatePluginParameters();
};