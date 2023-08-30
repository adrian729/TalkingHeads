/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TemplateDSPPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
	TemplateDSPPluginAudioProcessorEditor(TemplateDSPPluginAudioProcessor&, juce::AudioProcessorValueTreeState&, PrepareParameterInfo(&)[ControlID::countParams]);
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

	PrepareParameterInfo(&parameterInfoArr)[ControlID::countParams];

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TemplateDSPPluginAudioProcessorEditor)
};
