/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TemplateDSPPluginAudioProcessorEditor::TemplateDSPPluginAudioProcessorEditor(TemplateDSPPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& parametersAPVTS, PrepareParameterInfo(&parameterInfoArr)[ControlID::countParams])
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	parametersAPVTS(parametersAPVTS),
	parameterInfoArr(parameterInfoArr)
{
	setSize(400, 300);
}

TemplateDSPPluginAudioProcessorEditor::~TemplateDSPPluginAudioProcessorEditor()
{
}

//==============================================================================
void TemplateDSPPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

	g.setColour(juce::Colours::white);
	g.setFont(15.0f);
	bool val = ((juce::AudioParameterFloat*)parametersAPVTS.getParameter(parameterInfoArr[ControlID::gain].getParamID()))->get();
	g.drawFittedText(std::to_string(val), getLocalBounds(), juce::Justification::centred, 1);
}

void TemplateDSPPluginAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..
}
