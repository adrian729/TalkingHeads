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

std::array<juce::String, ControlID::countParams> PluginStateManager::paramIDs = createParamIDs();
std::array<juce::String, ControlID::countParams> PluginStateManager::paramNames = createParamNames();
std::array<ParameterDefinition, ControlID::countParams> PluginStateManager::parameterDefinitions = createParameterDefinitions();

std::array<juce::String, ControlID::countParams> PluginStateManager::createParamIDs()
{
	std::array<juce::String, ControlID::countParams> tmp_paramIDs;

	// --- stage 0: General -- Bypass ALL // Blend (dry/wet) // Pre Gain
	tmp_paramIDs[ControlID::bypass] = "bypass";
	tmp_paramIDs[ControlID::blend] = "blend";
	tmp_paramIDs[ControlID::preGain] = "preGain";

	// -- stage 1 -- HPF, LPF, 3 Band EQ
	// -- HPF
	tmp_paramIDs[ControlID::highpassBypass] = "highpassBypass";
	tmp_paramIDs[ControlID::highpassFreq] = "highpassFreq";
	tmp_paramIDs[ControlID::highpassSlope] = "highpassSlope";
	// -- LPF
	tmp_paramIDs[ControlID::lowpassBypass] = "lowpassBypass";
	tmp_paramIDs[ControlID::lowpassFreq] = "lowpassFreq";
	tmp_paramIDs[ControlID::lowpassSlope] = "lowpassSlope";
	// -- Band Filter 1
	tmp_paramIDs[ControlID::bandFilter1Bypass] = "bandFilter1Bypass";
	tmp_paramIDs[ControlID::bandFilter1PeakFreq] = "bandFilter1PeakFreq";
	tmp_paramIDs[ControlID::bandFilter1PeakGain] = "bandFilter1PeakGain";
	tmp_paramIDs[ControlID::bandFilter1PeakQ] = "bandFilter1PeakQ";
	// -- Band Filter 2
	tmp_paramIDs[ControlID::bandFilter2Bypass] = "bandFilter2Bypass";
	tmp_paramIDs[ControlID::bandFilter2PeakFreq] = "bandFilter2PeakFreq";
	tmp_paramIDs[ControlID::bandFilter2PeakGain] = "bandFilter2PeakGain";
	tmp_paramIDs[ControlID::bandFilter2PeakQ] = "bandFilter2PeakQ";
	// -- Band Filter 3
	tmp_paramIDs[ControlID::bandFilter3Bypass] = "bandFilter3Bypass";
	tmp_paramIDs[ControlID::bandFilter3PeakFreq] = "bandFilter3PeakFreq";
	tmp_paramIDs[ControlID::bandFilter3PeakGain] = "bandFilter3PeakGain";
	tmp_paramIDs[ControlID::bandFilter3PeakQ] = "bandFilter3PeakQ";

	// -- stage 2 -- 3 Band Compressor
	// -- Low Band Compressor
	tmp_paramIDs[ControlID::lowBandCompressorBypass] = "lowBandCompressorBypass";
	tmp_paramIDs[ControlID::lowBandCompressorThreshold] = "lowBandCompressorThreshold";
	tmp_paramIDs[ControlID::lowBandCompressorAttack] = "lowBandCompressorAttack";
	tmp_paramIDs[ControlID::lowBandCompressorRelease] = "lowBandCompressorRelease";
	tmp_paramIDs[ControlID::lowBandCompressorRatio] = "lowBandCompressorRatio";
	// -- Low-Mid Crossover
	tmp_paramIDs[ControlID::lowMidCrossoverFreq] = "lowMidCrossoverFreq";
	// -- Mid Band Compressor
	tmp_paramIDs[ControlID::midBandCompressorBypass] = "midBandCompressorBypass";
	tmp_paramIDs[ControlID::midBandCompressorThreshold] = "midBandCompressorThreshold";
	tmp_paramIDs[ControlID::midBandCompressorAttack] = "midBandCompressorAttack";
	tmp_paramIDs[ControlID::midBandCompressorRelease] = "midBandCompressorRelease";
	tmp_paramIDs[ControlID::midBandCompressorRatio] = "midBandCompressorRatio";
	// -- Mid-High Crossover
	tmp_paramIDs[ControlID::midHighCrossoverFreq] = "midHighCrossoverFreq";
	// -- High Band Compressor
	tmp_paramIDs[ControlID::highBandCompressorBypass] = "highBandCompressorBypass";
	tmp_paramIDs[ControlID::highBandCompressorThreshold] = "highBandCompressorThreshold";
	tmp_paramIDs[ControlID::highBandCompressorAttack] = "highBandCompressorAttack";
	tmp_paramIDs[ControlID::highBandCompressorRelease] = "highBandCompressorRelease";
	tmp_paramIDs[ControlID::highBandCompressorRatio] = "highBandCompressorRatio";

	return tmp_paramIDs;
}

std::array<juce::String, ControlID::countParams> PluginStateManager::createParamNames()
{
	std::array<juce::String, ControlID::countParams> tmp_paramNames;

	// --- stage 0: General -- Bypass ALL // Blend (dry/wet) // Pre Gain
	tmp_paramNames[ControlID::bypass] = "Bypass";
	tmp_paramNames[ControlID::blend] = "Blend";
	tmp_paramNames[ControlID::preGain] = "Pre-gain";

	// -- stage 1 -- HPF, LPF, 3 Band EQ
	// -- HPF
	tmp_paramNames[ControlID::highpassBypass] = "Highpass Bypass";
	tmp_paramNames[ControlID::highpassFreq] = "Highpass Freq";
	tmp_paramNames[ControlID::highpassSlope] = "Highpass Slope";
	// -- LPF
	tmp_paramNames[ControlID::lowpassBypass] = "Lowpass Bypass";
	tmp_paramNames[ControlID::lowpassFreq] = "Lowpass Freq";
	tmp_paramNames[ControlID::lowpassSlope] = "Lowpass Slope";
	// -- Band Filter 1
	tmp_paramNames[ControlID::bandFilter1Bypass] = "Band Filter 1 Bypass";
	tmp_paramNames[ControlID::bandFilter1PeakFreq] = "Band Filter 1 Peak Freq";
	tmp_paramNames[ControlID::bandFilter1PeakGain] = "Band Filter 1 Peak Gain";
	tmp_paramNames[ControlID::bandFilter1PeakQ] = "Band Filter 1 Q";
	// -- Band Filter 2
	tmp_paramNames[ControlID::bandFilter2Bypass] = "Band Filter 2 Bypass";
	tmp_paramNames[ControlID::bandFilter2PeakFreq] = "Band Filter 2 Peak Freq";
	tmp_paramNames[ControlID::bandFilter2PeakGain] = "Band Filter 2 Peak Gain";
	tmp_paramNames[ControlID::bandFilter2PeakQ] = "Band Filter 2 Q";
	// -- Band Filter 3
	tmp_paramNames[ControlID::bandFilter3Bypass] = "Band Filter 3 Bypass";
	tmp_paramNames[ControlID::bandFilter3PeakFreq] = "Band Filter 3 Peak Freq";
	tmp_paramNames[ControlID::bandFilter3PeakGain] = "Band Filter 3 Peak Gain";
	tmp_paramNames[ControlID::bandFilter3PeakQ] = "Band Filter 3 Q";

	// -- stage 2 -- 3 Band Compressor
	// -- Low Band Compressor
	tmp_paramNames[ControlID::lowBandCompressorBypass] = "Low Band Compressor Bypass";
	tmp_paramNames[ControlID::lowBandCompressorThreshold] = "Low Band Compressor Threshold";
	tmp_paramNames[ControlID::lowBandCompressorAttack] = "Low Band Compressor Attack";
	tmp_paramNames[ControlID::lowBandCompressorRelease] = "Low Band Compressor Release";
	tmp_paramNames[ControlID::lowBandCompressorRatio] = "Low Band Compressor Ratio";
	// -- Low-Mid Crossover
	tmp_paramNames[ControlID::lowMidCrossoverFreq] = "Low-Mid Crossover Freq";
	// -- Mid Band Compressor
	tmp_paramNames[ControlID::midBandCompressorBypass] = "Mid Band Compressor Bypass";
	tmp_paramNames[ControlID::midBandCompressorThreshold] = "Mid Band Compressor Threshold";
	tmp_paramNames[ControlID::midBandCompressorAttack] = "Mid Band Compressor Attack";
	tmp_paramNames[ControlID::midBandCompressorRelease] = "Mid Band Compressor Release";
	tmp_paramNames[ControlID::midBandCompressorRatio] = "Mid Band Compressor Ratio";
	// -- Mid-High Crossover
	tmp_paramNames[ControlID::midHighCrossoverFreq] = "Mid-High Crossover Freq";
	// -- High Band Compressor
	tmp_paramNames[ControlID::highBandCompressorBypass] = "High Band Compressor Bypass";
	tmp_paramNames[ControlID::highBandCompressorThreshold] = "High Band Compressor Threshold";
	tmp_paramNames[ControlID::highBandCompressorAttack] = "High Band Compressor Attack";
	tmp_paramNames[ControlID::highBandCompressorRelease] = "High Band Compressor Release";
	tmp_paramNames[ControlID::highBandCompressorRatio] = "High Band Compressor Ratio";

	return tmp_paramNames;
}

std::array<ParameterDefinition, ControlID::countParams> PluginStateManager::createParameterDefinitions()
{
	std::array<ParameterDefinition, ControlID::countParams> tmp_parameterDefinitions;

	auto gainRange = juce::NormalisableRange<float>(-24.f, 24.f, 0.01f);
	auto freqRange = juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.198893842f);
	auto filterQRange = juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f);
	auto thresholdRange = juce::NormalisableRange<float>(-60.f, 24.f, 1.f, 1.f);
	auto attackReleaseRange = juce::NormalisableRange<float>(5.f, 500.f, 1.f, 1.f);
	auto ratioRange = juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 0.4f);

	// -- General -- Bypass ALL // Blend (dry/wet) // Pre Gain
	tmp_parameterDefinitions[ControlID::bypass] = ParameterDefinition(
		ControlID::bypass,
		paramIDs[ControlID::bypass],
		1,
		paramNames[ControlID::bypass],
		false,
		"",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::blend] = ParameterDefinition(
		ControlID::blend,
		paramIDs[ControlID::blend],
		1,
		paramNames[ControlID::blend],
		juce::NormalisableRange<float>(0.f, 1.f, 0.01f),
		1.f,
		"",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::preGain] = ParameterDefinition(
		ControlID::preGain,
		paramIDs[ControlID::preGain],
		1,
		paramNames[ControlID::preGain],
		gainRange,
		0.f,
		"dB",
		SmoothingType::NoSmoothing // -- Gain DSP module already has smoothing
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
		paramIDs[ControlID::highpassBypass],
		1,
		paramNames[ControlID::highpassBypass],
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::highpassFreq] = ParameterDefinition(
		ControlID::highpassFreq,
		paramIDs[ControlID::highpassFreq],
		1,
		paramNames[ControlID::highpassFreq],
		freqRange,
		20.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::highpassSlope] = ParameterDefinition(
		ControlID::highpassSlope,
		paramIDs[ControlID::highpassSlope],
		1,
		paramNames[ControlID::highpassSlope],
		freqPassSlopeChoices
	);

	// -- LPF
	tmp_parameterDefinitions[ControlID::lowpassBypass] = ParameterDefinition(
		ControlID::lowpassBypass,
		paramIDs[ControlID::lowpassBypass],
		1,
		paramNames[ControlID::lowpassBypass],
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::lowpassFreq] = ParameterDefinition(
		ControlID::lowpassFreq,
		paramIDs[ControlID::lowpassFreq],
		1,
		paramNames[ControlID::lowpassFreq],
		freqRange,
		20000.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowpassSlope] = ParameterDefinition(
		ControlID::lowpassSlope,
		paramIDs[ControlID::lowpassSlope],
		1,
		paramNames[ControlID::lowpassSlope],
		freqPassSlopeChoices
	);

	// -- Band Filter 1
	tmp_parameterDefinitions[ControlID::bandFilter1Bypass] = ParameterDefinition(
		ControlID::bandFilter1Bypass,
		paramIDs[ControlID::bandFilter1Bypass],
		1,
		paramNames[ControlID::bandFilter1Bypass],
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::bandFilter1PeakFreq] = ParameterDefinition(
		ControlID::bandFilter1PeakFreq,
		paramIDs[ControlID::bandFilter1PeakFreq],
		1,
		paramNames[ControlID::bandFilter1PeakFreq],
		freqRange,
		750.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter1PeakGain] = ParameterDefinition(
		ControlID::bandFilter1PeakGain,
		paramIDs[ControlID::bandFilter1PeakGain],
		1,
		paramNames[ControlID::bandFilter1PeakGain],
		gainRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter1PeakQ] = ParameterDefinition(
		ControlID::bandFilter1PeakQ,
		paramIDs[ControlID::bandFilter1PeakQ],
		1,
		paramNames[ControlID::bandFilter1PeakQ],
		filterQRange,
		1.f,
		"",
		SmoothingType::Linear
	);

	// -- Band Filter 2
	tmp_parameterDefinitions[ControlID::bandFilter2Bypass] = ParameterDefinition(
		ControlID::bandFilter2Bypass,
		paramIDs[ControlID::bandFilter2Bypass],
		1,
		paramNames[ControlID::bandFilter2Bypass],
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::bandFilter2PeakFreq] = ParameterDefinition(
		ControlID::bandFilter2PeakFreq,
		paramIDs[ControlID::bandFilter2PeakFreq],
		1,
		paramNames[ControlID::bandFilter2PeakFreq],
		freqRange,
		2000.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter2PeakGain] = ParameterDefinition(
		ControlID::bandFilter2PeakGain,
		paramIDs[ControlID::bandFilter2PeakGain],
		1,
		paramNames[ControlID::bandFilter2PeakGain],
		gainRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter2PeakQ] = ParameterDefinition(
		ControlID::bandFilter2PeakQ,
		paramIDs[ControlID::bandFilter2PeakQ],
		1,
		paramNames[ControlID::bandFilter2PeakQ],
		filterQRange,
		1.f,
		"",
		SmoothingType::Linear
	);

	// -- Band Filter 3
	tmp_parameterDefinitions[ControlID::bandFilter3Bypass] = ParameterDefinition(
		ControlID::bandFilter3Bypass,
		paramIDs[ControlID::bandFilter3Bypass],
		1,
		paramNames[ControlID::bandFilter3Bypass],
		false,
		"",
		SmoothingType::NoSmoothing
	);

	tmp_parameterDefinitions[ControlID::bandFilter3PeakFreq] = ParameterDefinition(
		ControlID::bandFilter3PeakFreq,
		paramIDs[ControlID::bandFilter3PeakFreq],
		1,
		paramNames[ControlID::bandFilter3PeakFreq],
		freqRange,
		4000.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter3PeakGain] = ParameterDefinition(
		ControlID::bandFilter3PeakGain,
		paramIDs[ControlID::bandFilter3PeakGain],
		1,
		paramNames[ControlID::bandFilter3PeakGain],
		gainRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter3PeakQ] = ParameterDefinition(
		ControlID::bandFilter3PeakQ,
		paramIDs[ControlID::bandFilter3PeakQ],
		1,
		paramNames[ControlID::bandFilter3PeakQ],
		filterQRange,
		1.f,
		"",
		SmoothingType::Linear
	);

	// -- Second Stage -- 3 Band Compressor

	// -- Low Band Compressor
	tmp_parameterDefinitions[ControlID::lowBandCompressorBypass] = ParameterDefinition(
		ControlID::lowBandCompressorBypass,
		paramIDs[ControlID::lowBandCompressorBypass],
		false,
		paramNames[ControlID::lowBandCompressorBypass],
		false,
		"",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowBandCompressorThreshold] = ParameterDefinition(
		ControlID::lowBandCompressorThreshold,
		paramIDs[ControlID::lowBandCompressorThreshold],
		1,
		paramNames[ControlID::lowBandCompressorThreshold],
		thresholdRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowBandCompressorAttack] = ParameterDefinition(
		ControlID::lowBandCompressorAttack,
		paramIDs[ControlID::lowBandCompressorAttack],
		1,
		paramNames[ControlID::lowBandCompressorAttack],
		attackReleaseRange,
		50.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowBandCompressorRelease] = ParameterDefinition(
		ControlID::lowBandCompressorRelease,
		paramIDs[ControlID::lowBandCompressorRelease],
		1,
		paramNames[ControlID::lowBandCompressorRelease],
		attackReleaseRange,
		250.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowBandCompressorRatio] = ParameterDefinition(
		ControlID::lowBandCompressorRatio,
		paramIDs[ControlID::lowBandCompressorRatio],
		1,
		paramNames[ControlID::lowBandCompressorRatio],
		ratioRange,
		1.f,
		"",
		SmoothingType::Linear
	);

	// -- Low-Mid Crossover
	tmp_parameterDefinitions[ControlID::lowMidCrossoverFreq] = ParameterDefinition(
		ControlID::lowMidCrossoverFreq,
		paramIDs[ControlID::lowMidCrossoverFreq],
		1,
		paramNames[ControlID::lowMidCrossoverFreq],
		juce::NormalisableRange<float>(20.f, 999.f, 1.f, 0.198893842f),
		400.f,
		"Hz",
		SmoothingType::Linear
	);

	// -- Mid Band Compressor
	tmp_parameterDefinitions[ControlID::midBandCompressorBypass] = ParameterDefinition(
		ControlID::midBandCompressorBypass,
		paramIDs[ControlID::midBandCompressorBypass],
		1,
		paramNames[ControlID::midBandCompressorBypass],
		false,
		"",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::midBandCompressorThreshold] = ParameterDefinition(
		ControlID::midBandCompressorThreshold,
		paramIDs[ControlID::midBandCompressorThreshold],
		1,
		paramNames[ControlID::midBandCompressorThreshold],
		thresholdRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::midBandCompressorAttack] = ParameterDefinition(
		ControlID::midBandCompressorAttack,
		paramIDs[ControlID::midBandCompressorAttack],
		1,
		paramNames[ControlID::midBandCompressorAttack],
		attackReleaseRange,
		50.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::midBandCompressorRelease] = ParameterDefinition(
		ControlID::midBandCompressorRelease,
		paramIDs[ControlID::midBandCompressorRelease],
		1,
		paramNames[ControlID::midBandCompressorRelease],
		attackReleaseRange,
		250.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::midBandCompressorRatio] = ParameterDefinition(
		ControlID::midBandCompressorRatio,
		paramIDs[ControlID::midBandCompressorRatio],
		1,
		paramNames[ControlID::midBandCompressorRatio],
		ratioRange,
		1.f,
		"",
		SmoothingType::Linear
	);

	// -- Mid-High Crossover
	tmp_parameterDefinitions[ControlID::midHighCrossoverFreq] = ParameterDefinition(
		ControlID::midHighCrossoverFreq,
		paramIDs[ControlID::midHighCrossoverFreq],
		1,
		paramNames[ControlID::midHighCrossoverFreq],
		juce::NormalisableRange<float>(1000.f, 20000.f, 1.f, 0.198893842f),
		2000.f,
		"Hz",
		SmoothingType::Linear
	);

	// -- High Band Compressor
	tmp_parameterDefinitions[ControlID::highBandCompressorBypass] = ParameterDefinition(
		ControlID::highBandCompressorBypass,
		paramIDs[ControlID::highBandCompressorBypass],
		1,
		paramNames[ControlID::highBandCompressorBypass],
		false,
		"",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::highBandCompressorThreshold] = ParameterDefinition(
		ControlID::highBandCompressorThreshold,
		paramIDs[ControlID::highBandCompressorThreshold],
		1,
		paramNames[ControlID::highBandCompressorThreshold],
		thresholdRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::highBandCompressorAttack] = ParameterDefinition(
		ControlID::highBandCompressorAttack,
		paramIDs[ControlID::highBandCompressorAttack],
		1,
		paramNames[ControlID::highBandCompressorAttack],
		attackReleaseRange,
		50.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::highBandCompressorRelease] = ParameterDefinition(
		ControlID::highBandCompressorRelease,
		paramIDs[ControlID::highBandCompressorRelease],
		1,
		paramNames[ControlID::highBandCompressorRelease],
		attackReleaseRange,
		250.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::highBandCompressorRatio] = ParameterDefinition(
		ControlID::highBandCompressorRatio,
		paramIDs[ControlID::highBandCompressorRatio],
		1,
		paramNames[ControlID::highBandCompressorRatio],
		ratioRange,
		1.f,
		"",
		SmoothingType::Linear
	);

	return tmp_parameterDefinitions;
}
