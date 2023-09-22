/*
  ==============================================================================

	PluginStateManager.cpp
	Created: 1 Sep 2023 1:42:27pm
	Author:  Brutus729

  ==============================================================================
*/

#include "PluginStateManager.h"

//==============================================================================
// -- CONSTRUCTORS
PluginStateManager::PluginStateManager()
{
}

PluginStateManager::~PluginStateManager()
{
}

//==============================================================================
std::array<ParameterDefinition, ControlID::countParams>& PluginStateManager::getParameterDefinitions()
{
	return parameterDefinitions;
}

//==============================================================================
std::array<ParameterDefinition, ControlID::countParams> PluginStateManager::createParameterDefinitions()
{
	std::array<ParameterDefinition, ControlID::countParams> tmp_parameterDefinitions;

	// -- General -- Bypass ALL // Blend (dry/wet) // Pre Gain
	tmp_parameterDefinitions[ControlID::bypass] = ParameterDefinition(
		ControlID::bypass,
		getParamID(ControlID::bypass),
		1,
		"Bypass",
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::blend] = ParameterDefinition(
		ControlID::blend,
		getParamID(ControlID::blend),
		1,
		"Blend",
		juce::NormalisableRange<float>(0.f, 1.f, 0.01f),
		1.f,
		"",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::preGain] = ParameterDefinition(
		ControlID::preGain,
		getParamID(ControlID::preGain),
		1,
		"Pre-gain",
		juce::NormalisableRange<float>(-24.f, 24.f, 0.01f),
		0.f,
		"dB"
	);

	// -- First Stage -- Gain // HPF, LPF, 3 Band EQ // 3 Band Compressor

	juce::StringArray freqPassSlopeChoices;
	for (int i = 0; i < 4; ++i)
	{
		juce::String str;
		str << (12 + i * 12);
		str << " db/Oct";
		freqPassSlopeChoices.add(str);
	}

	// -- HPF
	tmp_parameterDefinitions[ControlID::highpassBypass] = ParameterDefinition(
		ControlID::highpassBypass,
		getParamID(ControlID::highpassBypass),
		1,
		"Highpass Bypass",
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::highpassFreq] = ParameterDefinition(
		ControlID::highpassFreq,
		getParamID(ControlID::highpassFreq),
		1,
		"Highpass Freq",
		juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
		20.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::highpassSlope] = ParameterDefinition(
		ControlID::highpassSlope,
		getParamID(ControlID::highpassSlope),
		1,
		"Highpass Slope",
		freqPassSlopeChoices
	);

	// -- LPF
	tmp_parameterDefinitions[ControlID::lowpassBypass] = ParameterDefinition(
		ControlID::lowpassBypass,
		getParamID(ControlID::lowpassBypass),
		1,
		"Lowpass Bypass",
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::lowpassFreq] = ParameterDefinition(
		ControlID::lowpassFreq,
		getParamID(ControlID::lowpassFreq),
		1,
		"Lowpass Freq",
		juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
		20000.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowpassSlope] = ParameterDefinition(
		ControlID::lowpassSlope,
		getParamID(ControlID::lowpassSlope),
		1,
		"Lowpass Slope",
		freqPassSlopeChoices
	);

	// -- Band Filter 1
	tmp_parameterDefinitions[ControlID::bandFilter1Bypass] = ParameterDefinition(
		ControlID::bandFilter1Bypass,
		getParamID(ControlID::bandFilter1Bypass),
		1,
		"Band Filter 1 Bypass",
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::bandFilter1PeakFreq] = ParameterDefinition(
		ControlID::bandFilter1PeakFreq,
		getParamID(ControlID::bandFilter1PeakFreq),
		1,
		"Band Filter 1 Peak Freq",
		juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
		750.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter1PeakGain] = ParameterDefinition(
		ControlID::bandFilter1PeakGain,
		getParamID(ControlID::bandFilter1PeakGain),
		1,
		"Band Filter 1 Peak Gain",
		juce::NormalisableRange<float>(-24.f, 24.f, 0.01f),
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter1PeakQ] = ParameterDefinition(
		ControlID::bandFilter1PeakQ,
		getParamID(ControlID::bandFilter1PeakQ),
		1,
		"Band Filter 1 Q",
		juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
		1.f,
		"",
		SmoothingType::Linear
	);

	// -- Band Filter 2
	tmp_parameterDefinitions[ControlID::bandFilter2Bypass] = ParameterDefinition(
		ControlID::bandFilter2Bypass,
		getParamID(ControlID::bandFilter2Bypass),
		1,
		"Band Filter 2 Bypass",
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::bandFilter2PeakFreq] = ParameterDefinition(
		ControlID::bandFilter2PeakFreq,
		getParamID(ControlID::bandFilter2PeakFreq),
		1,
		"Band Filter 2 Peak Freq",
		juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
		2000.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter2PeakGain] = ParameterDefinition(
		ControlID::bandFilter2PeakGain,
		getParamID(ControlID::bandFilter2PeakGain),
		1,
		"Band Filter 2 Peak Gain",
		juce::NormalisableRange<float>(-24.f, 24.f, 0.01f),
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter2PeakQ] = ParameterDefinition(
		ControlID::bandFilter2PeakQ,
		getParamID(ControlID::bandFilter2PeakQ),
		1,
		"Band Filter 2 Q",
		juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
		1.f,
		"",
		SmoothingType::Linear
	);

	// -- Band Filter 3
	tmp_parameterDefinitions[ControlID::bandFilter3Bypass] = ParameterDefinition(
		ControlID::bandFilter3Bypass,
		getParamID(ControlID::bandFilter3Bypass),
		1,
		"Band Filter 3 Bypass",
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::bandFilter3PeakFreq] = ParameterDefinition(
		ControlID::bandFilter3PeakFreq,
		getParamID(ControlID::bandFilter3PeakFreq),
		1,
		"Band Filter 3 Peak Freq",
		juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
		4000.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter3PeakGain] = ParameterDefinition(
		ControlID::bandFilter3PeakGain,
		getParamID(ControlID::bandFilter3PeakGain),
		1,
		"Band Filter 3 Peak Gain",
		juce::NormalisableRange<float>(-24.f, 24.f, 0.01f),
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter3PeakQ] = ParameterDefinition(
		ControlID::bandFilter3PeakQ,
		getParamID(ControlID::bandFilter3PeakQ),
		1,
		"Band Filter 3 Q",
		juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
		1.f,
		"",
		SmoothingType::Linear
	);

	return tmp_parameterDefinitions;
}
