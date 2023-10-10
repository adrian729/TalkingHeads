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
std::array<juce::String, ControlID::countParams>& PluginStateManager::getParameterIDs()
{
	return parameterIDs;
}

std::array<juce::String, ControlID::countParams>& PluginStateManager::getParameterNames()
{
	return parameterNames;
}

std::array<ParameterDefinition, ControlID::countParams>& PluginStateManager::getParameterDefinitions()
{
	return parameterDefinitions;
}

//==============================================================================

std::array<juce::String, ControlID::countParams> PluginStateManager::parameterIDs = createParamIDs();
std::array<juce::String, ControlID::countParams> PluginStateManager::parameterNames = createParamNames();
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

	// -- Phaser
	tmp_paramIDs[ControlID::phaserBypass] = "phaserBypass";
	tmp_paramIDs[ControlID::phaserRate] = "phaserRate";
	tmp_paramIDs[ControlID::phaserDepth] = "phaserDepth";
	tmp_paramIDs[ControlID::phaserCentreFrequency] = "phaserCentreFrequency";
	tmp_paramIDs[ControlID::phaserFeedback] = "phaserFeedback";
	tmp_paramIDs[ControlID::phaserMix] = "phaserMix";

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
	// -- Low-Mid Crossover
	tmp_paramNames[ControlID::lowMidCrossoverFreq] = "Low-Mid Crossover Freq";
	// -- Mid-High Crossover
	tmp_paramNames[ControlID::midHighCrossoverFreq] = "Mid-High Crossover Freq";
	// -- Low Band Compressor
	tmp_paramNames[ControlID::lowBandCompressorBypass] = "Low Band Compressor Bypass";
	tmp_paramNames[ControlID::lowBandCompressorThreshold] = "Low Band Compressor Threshold";
	tmp_paramNames[ControlID::lowBandCompressorAttack] = "Low Band Compressor Attack";
	tmp_paramNames[ControlID::lowBandCompressorRelease] = "Low Band Compressor Release";
	tmp_paramNames[ControlID::lowBandCompressorRatio] = "Low Band Compressor Ratio";
	// -- Mid Band Compressor
	tmp_paramNames[ControlID::midBandCompressorBypass] = "Mid Band Compressor Bypass";
	tmp_paramNames[ControlID::midBandCompressorThreshold] = "Mid Band Compressor Threshold";
	tmp_paramNames[ControlID::midBandCompressorAttack] = "Mid Band Compressor Attack";
	tmp_paramNames[ControlID::midBandCompressorRelease] = "Mid Band Compressor Release";
	tmp_paramNames[ControlID::midBandCompressorRatio] = "Mid Band Compressor Ratio";
	// -- High Band Compressor
	tmp_paramNames[ControlID::highBandCompressorBypass] = "High Band Compressor Bypass";
	tmp_paramNames[ControlID::highBandCompressorThreshold] = "High Band Compressor Threshold";
	tmp_paramNames[ControlID::highBandCompressorAttack] = "High Band Compressor Attack";
	tmp_paramNames[ControlID::highBandCompressorRelease] = "High Band Compressor Release";
	tmp_paramNames[ControlID::highBandCompressorRatio] = "High Band Compressor Ratio";

	// -- Phaser
	tmp_paramNames[ControlID::phaserBypass] = "Phaser Bypass";
	tmp_paramNames[ControlID::phaserRate] = "Phaser Rate";
	tmp_paramNames[ControlID::phaserDepth] = "Phaser Depth";
	tmp_paramNames[ControlID::phaserCentreFrequency] = "Phaser Centre Frequency";
	tmp_paramNames[ControlID::phaserFeedback] = "Phaser Feedback";
	tmp_paramNames[ControlID::phaserMix] = "Phaser Mix";

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

	//==============================================================================
	// -- Bypass ALL
	tmp_parameterDefinitions[ControlID::bypass] = ParameterDefinition(
		ControlID::bypass,
		parameterIDs[ControlID::bypass],
		V1_0_0,
		parameterNames[ControlID::bypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);
	// -- Blend (dry/wet)
	tmp_parameterDefinitions[ControlID::blend] = ParameterDefinition(
		ControlID::blend,
		parameterIDs[ControlID::blend],
		V1_0_0,
		parameterNames[ControlID::blend],
		juce::NormalisableRange<float>(0.f, 1.f, 0.01f),
		1.f,
		"",
		SmoothingType::Linear
	);
	// -- Pre Gain
	tmp_parameterDefinitions[ControlID::preGain] = ParameterDefinition(
		ControlID::preGain,
		parameterIDs[ControlID::preGain],
		V1_0_0,
		parameterNames[ControlID::preGain],
		gainRange,
		0.f,
		"dB",
		SmoothingType::NoSmoothing // -- Gain DSP module already has smoothing
	);

	//==============================================================================
	// -- Multi Band EQ -- HPF, LPF, 3 Band 
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
		parameterIDs[ControlID::highpassBypass],
		V1_0_0,
		parameterNames[ControlID::highpassBypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);

	tmp_parameterDefinitions[ControlID::highpassFreq] = ParameterDefinition(
		ControlID::highpassFreq,
		parameterIDs[ControlID::highpassFreq],
		V1_0_0,
		parameterNames[ControlID::highpassFreq],
		freqRange,
		20.f,
		"Hz",
		SmoothingType::Linear,
		0.01f
	);

	tmp_parameterDefinitions[ControlID::highpassSlope] = ParameterDefinition(
		ControlID::highpassSlope,
		parameterIDs[ControlID::highpassSlope],
		V1_0_0,
		parameterNames[ControlID::highpassSlope],
		freqPassSlopeChoices
	);
	// -- LPF
	tmp_parameterDefinitions[ControlID::lowpassBypass] = ParameterDefinition(
		ControlID::lowpassBypass,
		parameterIDs[ControlID::lowpassBypass],
		V1_0_0,
		parameterNames[ControlID::lowpassBypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);

	tmp_parameterDefinitions[ControlID::lowpassFreq] = ParameterDefinition(
		ControlID::lowpassFreq,
		parameterIDs[ControlID::lowpassFreq],
		V1_0_0,
		parameterNames[ControlID::lowpassFreq],
		freqRange,
		20000.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowpassSlope] = ParameterDefinition(
		ControlID::lowpassSlope,
		parameterIDs[ControlID::lowpassSlope],
		V1_0_0,
		parameterNames[ControlID::lowpassSlope],
		freqPassSlopeChoices
	);
	// -- Band Filter 1
	tmp_parameterDefinitions[ControlID::bandFilter1Bypass] = ParameterDefinition(
		ControlID::bandFilter1Bypass,
		parameterIDs[ControlID::bandFilter1Bypass],
		V1_0_0,
		parameterNames[ControlID::bandFilter1Bypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);

	tmp_parameterDefinitions[ControlID::bandFilter1PeakFreq] = ParameterDefinition(
		ControlID::bandFilter1PeakFreq,
		parameterIDs[ControlID::bandFilter1PeakFreq],
		V1_0_0,
		parameterNames[ControlID::bandFilter1PeakFreq],
		freqRange,
		750.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter1PeakGain] = ParameterDefinition(
		ControlID::bandFilter1PeakGain,
		parameterIDs[ControlID::bandFilter1PeakGain],
		V1_0_0,
		parameterNames[ControlID::bandFilter1PeakGain],
		gainRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter1PeakQ] = ParameterDefinition(
		ControlID::bandFilter1PeakQ,
		parameterIDs[ControlID::bandFilter1PeakQ],
		V1_0_0,
		parameterNames[ControlID::bandFilter1PeakQ],
		filterQRange,
		1.f,
		"",
		SmoothingType::Linear
	);
	// -- Band Filter 2
	tmp_parameterDefinitions[ControlID::bandFilter2Bypass] = ParameterDefinition(
		ControlID::bandFilter2Bypass,
		parameterIDs[ControlID::bandFilter2Bypass],
		V1_0_0,
		parameterNames[ControlID::bandFilter2Bypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);

	tmp_parameterDefinitions[ControlID::bandFilter2PeakFreq] = ParameterDefinition(
		ControlID::bandFilter2PeakFreq,
		parameterIDs[ControlID::bandFilter2PeakFreq],
		V1_0_0,
		parameterNames[ControlID::bandFilter2PeakFreq],
		freqRange,
		2000.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter2PeakGain] = ParameterDefinition(
		ControlID::bandFilter2PeakGain,
		parameterIDs[ControlID::bandFilter2PeakGain],
		V1_0_0,
		parameterNames[ControlID::bandFilter2PeakGain],
		gainRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter2PeakQ] = ParameterDefinition(
		ControlID::bandFilter2PeakQ,
		parameterIDs[ControlID::bandFilter2PeakQ],
		V1_0_0,
		parameterNames[ControlID::bandFilter2PeakQ],
		filterQRange,
		1.f,
		"",
		SmoothingType::Linear
	);
	// -- Band Filter 3
	tmp_parameterDefinitions[ControlID::bandFilter3Bypass] = ParameterDefinition(
		ControlID::bandFilter3Bypass,
		parameterIDs[ControlID::bandFilter3Bypass],
		V1_0_0,
		parameterNames[ControlID::bandFilter3Bypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);

	tmp_parameterDefinitions[ControlID::bandFilter3PeakFreq] = ParameterDefinition(
		ControlID::bandFilter3PeakFreq,
		parameterIDs[ControlID::bandFilter3PeakFreq],
		V1_0_0,
		parameterNames[ControlID::bandFilter3PeakFreq],
		freqRange,
		4000.f,
		"Hz",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter3PeakGain] = ParameterDefinition(
		ControlID::bandFilter3PeakGain,
		parameterIDs[ControlID::bandFilter3PeakGain],
		V1_0_0,
		parameterNames[ControlID::bandFilter3PeakGain],
		gainRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::bandFilter3PeakQ] = ParameterDefinition(
		ControlID::bandFilter3PeakQ,
		parameterIDs[ControlID::bandFilter3PeakQ],
		V1_0_0,
		parameterNames[ControlID::bandFilter3PeakQ],
		filterQRange,
		1.f,
		"",
		SmoothingType::Linear
	);

	//==============================================================================
	// -- Multi Band Compressor
	// -- Low-Mid Crossover
	tmp_parameterDefinitions[ControlID::lowMidCrossoverFreq] = ParameterDefinition(
		ControlID::lowMidCrossoverFreq,
		parameterIDs[ControlID::lowMidCrossoverFreq],
		V1_0_0,
		parameterNames[ControlID::lowMidCrossoverFreq],
		juce::NormalisableRange<float>(20.f, 999.f, 1.f, 0.198893842f),
		400.f,
		"Hz",
		SmoothingType::Linear
	);
	// -- Mid-High Crossover
	tmp_parameterDefinitions[ControlID::midHighCrossoverFreq] = ParameterDefinition(
		ControlID::midHighCrossoverFreq,
		parameterIDs[ControlID::midHighCrossoverFreq],
		V1_0_0,
		parameterNames[ControlID::midHighCrossoverFreq],
		juce::NormalisableRange<float>(1000.f, 20000.f, 1.f, 0.198893842f),
		2000.f,
		"Hz",
		SmoothingType::Linear
	);
	// -- Low Band Compressor
	tmp_parameterDefinitions[ControlID::lowBandCompressorBypass] = ParameterDefinition(
		ControlID::lowBandCompressorBypass,
		parameterIDs[ControlID::lowBandCompressorBypass],
		V1_0_0,
		parameterNames[ControlID::lowBandCompressorBypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);

	tmp_parameterDefinitions[ControlID::lowBandCompressorThreshold] = ParameterDefinition(
		ControlID::lowBandCompressorThreshold,
		parameterIDs[ControlID::lowBandCompressorThreshold],
		V1_0_0,
		parameterNames[ControlID::lowBandCompressorThreshold],
		thresholdRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowBandCompressorAttack] = ParameterDefinition(
		ControlID::lowBandCompressorAttack,
		parameterIDs[ControlID::lowBandCompressorAttack],
		V1_0_0,
		parameterNames[ControlID::lowBandCompressorAttack],
		attackReleaseRange,
		50.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowBandCompressorRelease] = ParameterDefinition(
		ControlID::lowBandCompressorRelease,
		parameterIDs[ControlID::lowBandCompressorRelease],
		V1_0_0,
		parameterNames[ControlID::lowBandCompressorRelease],
		attackReleaseRange,
		250.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::lowBandCompressorRatio] = ParameterDefinition(
		ControlID::lowBandCompressorRatio,
		parameterIDs[ControlID::lowBandCompressorRatio],
		V1_0_0,
		parameterNames[ControlID::lowBandCompressorRatio],
		ratioRange,
		1.f,
		"",
		SmoothingType::Linear
	);
	// -- Mid Band Compressor
	tmp_parameterDefinitions[ControlID::midBandCompressorBypass] = ParameterDefinition(
		ControlID::midBandCompressorBypass,
		parameterIDs[ControlID::midBandCompressorBypass],
		V1_0_0,
		parameterNames[ControlID::midBandCompressorBypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);

	tmp_parameterDefinitions[ControlID::midBandCompressorThreshold] = ParameterDefinition(
		ControlID::midBandCompressorThreshold,
		parameterIDs[ControlID::midBandCompressorThreshold],
		V1_0_0,
		parameterNames[ControlID::midBandCompressorThreshold],
		thresholdRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::midBandCompressorAttack] = ParameterDefinition(
		ControlID::midBandCompressorAttack,
		parameterIDs[ControlID::midBandCompressorAttack],
		V1_0_0,
		parameterNames[ControlID::midBandCompressorAttack],
		attackReleaseRange,
		50.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::midBandCompressorRelease] = ParameterDefinition(
		ControlID::midBandCompressorRelease,
		parameterIDs[ControlID::midBandCompressorRelease],
		V1_0_0,
		parameterNames[ControlID::midBandCompressorRelease],
		attackReleaseRange,
		250.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::midBandCompressorRatio] = ParameterDefinition(
		ControlID::midBandCompressorRatio,
		parameterIDs[ControlID::midBandCompressorRatio],
		V1_0_0,
		parameterNames[ControlID::midBandCompressorRatio],
		ratioRange,
		1.f,
		"",
		SmoothingType::Linear
	);
	// -- High Band Compressor
	tmp_parameterDefinitions[ControlID::highBandCompressorBypass] = ParameterDefinition(
		ControlID::highBandCompressorBypass,
		parameterIDs[ControlID::highBandCompressorBypass],
		V1_0_0,
		parameterNames[ControlID::highBandCompressorBypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);

	tmp_parameterDefinitions[ControlID::highBandCompressorThreshold] = ParameterDefinition(
		ControlID::highBandCompressorThreshold,
		parameterIDs[ControlID::highBandCompressorThreshold],
		V1_0_0,
		parameterNames[ControlID::highBandCompressorThreshold],
		thresholdRange,
		0.f,
		"dB",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::highBandCompressorAttack] = ParameterDefinition(
		ControlID::highBandCompressorAttack,
		parameterIDs[ControlID::highBandCompressorAttack],
		V1_0_0,
		parameterNames[ControlID::highBandCompressorAttack],
		attackReleaseRange,
		50.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::highBandCompressorRelease] = ParameterDefinition(
		ControlID::highBandCompressorRelease,
		parameterIDs[ControlID::highBandCompressorRelease],
		V1_0_0,
		parameterNames[ControlID::highBandCompressorRelease],
		attackReleaseRange,
		250.f,
		"ms",
		SmoothingType::Linear
	);

	tmp_parameterDefinitions[ControlID::highBandCompressorRatio] = ParameterDefinition(
		ControlID::highBandCompressorRatio,
		parameterIDs[ControlID::highBandCompressorRatio],
		V1_0_0,
		parameterNames[ControlID::highBandCompressorRatio],
		ratioRange,
		1.f,
		"",
		SmoothingType::Linear
	);

	//==============================================================================
	// -- Phaser
	tmp_parameterDefinitions[ControlID::phaserBypass] = ParameterDefinition(
		ControlID::phaserBypass,
		parameterIDs[ControlID::phaserBypass],
		V1_0_0,
		parameterNames[ControlID::phaserBypass],
		false,
		"",
		SmoothingType::Linear,
		0.01f
	);

	// -- Phaser LFO
	// -- LFO rate -- (0, 100) Hz
	tmp_parameterDefinitions[ControlID::phaserRate] = ParameterDefinition(
		ControlID::phaserRate,
		parameterIDs[ControlID::phaserRate],
		V1_0_0,
		parameterNames[ControlID::phaserRate],
		juce::NormalisableRange<float>(0.f, 100.f, 0.001f, 0.35f), // TODO: check skew factor
		1.f,
		"",
		SmoothingType::Linear
	);

	// -- LFO depth -- (0, 1)
	tmp_parameterDefinitions[ControlID::phaserDepth] = ParameterDefinition(
		ControlID::phaserDepth,
		parameterIDs[ControlID::phaserDepth],
		V1_0_0,
		parameterNames[ControlID::phaserDepth],
		juce::NormalisableRange<float>(0.f, 1.f, 0.001f, 0.5f),
		0.5f,
		"",
		SmoothingType::Linear
	);

	// -- Phaser Filter -- Centre Frequency -- (20, 20000) Hz
	tmp_parameterDefinitions[ControlID::phaserCentreFrequency] = ParameterDefinition(
		ControlID::phaserCentreFrequency,
		parameterIDs[ControlID::phaserCentreFrequency],
		V1_0_0,
		parameterNames[ControlID::phaserCentreFrequency],
		freqRange,
		1300.f,
		"Hz",
		SmoothingType::Linear
	);

	// -- Phaser other params
	// -- Feedback -- (-1, 1)
	tmp_parameterDefinitions[ControlID::phaserFeedback] = ParameterDefinition(
		ControlID::phaserFeedback,
		parameterIDs[ControlID::phaserFeedback],
		V1_0_0,
		parameterNames[ControlID::phaserFeedback],
		juce::NormalisableRange<float>(-1.f, 1.f, 0.01f),
		0.f,
		"",
		SmoothingType::Linear
	);

	// -- Mix -- (0, 1)
	tmp_parameterDefinitions[ControlID::phaserMix] = ParameterDefinition(
		ControlID::phaserMix,
		parameterIDs[ControlID::phaserMix],
		V1_0_0,
		parameterNames[ControlID::phaserMix],
		juce::NormalisableRange<float>(0.f, 1.f, 0.01f),
		0.5f,
		"",
		SmoothingType::Linear
	);

	return tmp_parameterDefinitions;
}
