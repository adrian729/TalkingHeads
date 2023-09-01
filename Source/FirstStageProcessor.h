/*
  ==============================================================================

	FirstStageProcessor.h
	Created: 1 Sep 2023 1:02:02pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class FirstStageProcessor : public juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	FirstStageProcessor();
	~FirstStageProcessor();

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

private:

};
