/*
  ==============================================================================

	Imager.cpp
	Created: 20 Oct 2023 1:17:45am
	Author:  Brutus729

	Stereo Imager to add width to the stereo field

	Adding stereo space to mono audio using Haas effect:

	1st approach:
	- Create auxiliar signal with 2-50ms delay
	- Pan original and auxiliar signals to opposite directions
	- Play with panning/level of the signals
	* width: pan distance between both signals/center: panning center/gain: gain for each signal/delay time: delay time for auxiliar signal

	2nd approach:
	- Keep original signal in center
	- Create auxiliar signal with 2-50ms delay
	- Add auxiliar signal to left/right channels but with inverted phase
	* This way, the center is kept and the stereo field is widened. Also when the signal is summed to mono, the auxiliar signal is cancelled out

	3rd approach: play with mid-side processing (still need to check)

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
		ControlID delayTimeID,
		ControlID crossoverFreqID,
		ControlID imagerTypeID
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

	enum StereoIDs
	{
		left,
		right
	};

	// --- Object parameters management and information
	std::shared_ptr<PluginStateManager> stateManager;

	ControlID bypassID{ ControlID::countParams };
	std::array<ControlID, countSignals> gainIDs;
	ControlID widthID{ ControlID::countParams };
	ControlID centerID{ ControlID::countParams };
	ControlID delayTimeID{ ControlID::countParams };
	ControlID crossoverFreqID{ ControlID::countParams };
	ControlID imagerTypeID{ ControlID::countParams };

	//==============================================================================
	// --- Object member variables
	int sampleRate{ 0 };

	float bypass{ 0.f };
	float isBypassed{ false };

	// -- Imager
	std::array<float, SignalIDs::countSignals> gains{ 0.5f, 0.5f };
	float width{ 0.f };
	float center{ 0.f };
	std::array<float, SignalIDs::countSignals> panningCoefficients{ 0.f, 0.f };
	float delayTime{ 0.f };

	juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> stereoImagerDelayLine{ 192000 };

	// -- Crossover -- allows to define from which freq the imager widens the stereo field
	float crossoverFreq{ 0.f };

	enum FilterIDs
	{
		lowpass,
		highpass,
		//==============================================================================
		countFilters
	};

	using Filter = juce::dsp::LinkwitzRileyFilter<float>;
	std::array<Filter, FilterIDs::countFilters> filters;
	juce::AudioBuffer<float> lowpassBuffer;
	juce::dsp::AudioBlock<float> lowpassBlock;

	enum ImagerTypes
	{
		haas,
		haasMono,
		haasMidSide,
		//==============================================================================
		countImagerTypes
	};

	ImagerTypes imagerType{ ImagerTypes::countImagerTypes };

	//==============================================================================
	float getDelayTimeInSamples();

	//==============================================================================
	void preProcess();
	void calculatePanningCoefficients();
};