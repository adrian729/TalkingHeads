/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
//#include "ParameterDefinition.h"

//==============================================================================
/**
*/
class TemplateDSPPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
	//TemplateDSPPluginAudioProcessorEditor(TemplateDSPPluginAudioProcessor&, juce::AudioProcessorValueTreeState&, ParameterDefinition(&)[ControlID::countParams]);
	TemplateDSPPluginAudioProcessorEditor(TemplateDSPPluginAudioProcessor&, juce::AudioProcessorValueTreeState&);
	~TemplateDSPPluginAudioProcessorEditor() override;

	//==============================================================================
	void paint(juce::Graphics&) override;
	void resized() override;

private:
	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	TemplateDSPPluginAudioProcessor& audioProcessor;

	using APVTS = juce::AudioProcessorValueTreeState;
	APVTS& parametersAPVTS;

	//ParameterDefinition(&parameterDefinitions)[ControlID::countParams];

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TemplateDSPPluginAudioProcessorEditor)
};
