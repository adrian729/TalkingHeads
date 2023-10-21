/*
  ==============================================================================

	Imager.cpp
	Created: 20 Oct 2023 1:17:45am
	Author:  Brutus729

	Stereo Imager to add width to the stereo field

	Adding stereo space to mono audio using Haas effect:

	1. Create auxiliar buffer as a copy of the input buffer
	2. Pan original and auxiliar buffers (preferably to opposite directions)
	3. Delay auxiliar buffer (between 2 and 50 ms)
	4. Change gain of original and auxiliar buffers


	Mid-side processing to add width:
	'width' is the stretch factor of the stereo field:
		width < 1: decrease in stereo width
		width = 1: no change
		width > 1: increase in stereo width
		width = 0: mono

		// calculate scale coefficient
		coef_S = width*0.5;

		// then do this per sample
		m = (in_left  + in_right)*0.5;
		s = (in_right - in_left )*coef_S;

		out_left  = m - s;
		out_right = m + s;

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "PluginStateManager.h"

//==============================================================================
class Imager : public juce::dsp::ProcessorBase
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	Imager(
		std::shared_ptr<PluginStateManager> stateManager,
		ControlID bypassID,
		ControlID originalGainID,
		ControlID auxiliarGainID,
		ControlID widthID,
		ControlID centerID,
		ControlID delayTimeID
	);
	~Imager();

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) override;
	void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
	void reset() override;

	//==============================================================================
	float getLatency();

	//==============================================================================
	void setSampleRate(int newSampleRate);

private:
	//==============================================================================
	const int MONO_CHANNEL{ 0 };
	const int LEFT_CHANNEL{ 0 };
	const int RIGHT_CHANNEL{ 1 };

	enum SignalIDs
	{
		original,
		auxiliar,
		//==============================================================================
		countSignals
	};

	// --- Object parameters management and information
	std::shared_ptr<PluginStateManager> stateManager;

	ControlID bypassID{ ControlID::countParams };
	std::array<ControlID, countSignals> gainIDs;
	ControlID widthID{ ControlID::countParams };
	ControlID centerID{ ControlID::countParams };
	ControlID delayTimeID{ ControlID::countParams };

	//==============================================================================
	// --- Object member variables
	int sampleRate{ 0 };

	float bypass{ 0.f };
	float isBypassed{ false };
	std::array<float, SignalIDs::countSignals> gains{ 0.5f, 0.5f };
	float width{ 0.f };
	float center{ 0.f };
	std::array<float, SignalIDs::countSignals> panningCoefficients{ 0.f, 0.f };
	float delayTime{ 0.f };

	juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> stereoImagerDelayLine{ 192000 };

	//==============================================================================
	float getDelayTimeInSamples();

	//==============================================================================
	void preProcess();
	void calculatePanningCoefficients();
};