/*
  ==============================================================================

	parameterTypes.h
	Created: 30 Aug 2023 12:22:56pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

//==============================================================================
// --- MACROS enum
//------------------------------------------------------------------------------
#define enumToInt(ENUM) static_cast<int>(ENUM)
#define intToEnum(INT, ENUM) static_cast<ENUM>(INT)
#define compareEnumToInt(ENUM, INT) (enumToInt(ENUM) == INT)
#define compareIntToEnum(INT, ENUM) (INT == enumToInt(ENUM))

enum class SmoothingType
{
	NoSmoothing,
	Linear,
	Multiplicative
};

//==============================================================================
// --- VERSIONS
//==============================================================================
enum VersionIDs
{
	NoVersion,
	// --- current version
	V1_0_0,
};

//==============================================================================
// --- CONTROL IDs -- for param definitions array access
//==============================================================================
enum ControlID
{
	// --- stage 0: General -- Bypass ALL // Blend (dry/wet) // Pre Gain
	bypass,
	blend,
	// -- Pre Gain
	preGain,

	// -- stage 1 -- HPF, LPF, 3 Band EQ
	// -- HPF
	highpassBypass,
	highpassFreq,
	highpassSlope,
	// -- LPF
	lowpassBypass,
	lowpassFreq,
	lowpassSlope,
	// -- Band Filter 1
	bandFilter1Bypass,
	bandFilter1PeakFreq,
	bandFilter1PeakGain,
	bandFilter1PeakQ,
	// -- Band Filter 2
	bandFilter2Bypass,
	bandFilter2PeakFreq,
	bandFilter2PeakGain,
	bandFilter2PeakQ,
	// -- Band Filter 3
	bandFilter3Bypass,
	bandFilter3PeakFreq,
	bandFilter3PeakGain,
	bandFilter3PeakQ,

	// -- stage 2 -- 3 Band Compressor
	compressorBypass,
	// -- Crossover
	lowMidCrossoverFreq,
	midHighCrossoverFreq,
	// -- Low Band Compressor
	lowBandCompressorBypass,
	lowBandCompressorThreshold,
	lowBandCompressorAttack,
	lowBandCompressorRelease,
	lowBandCompressorRatio,
	// -- Mid Band Compressor
	midBandCompressorBypass,
	midBandCompressorThreshold,
	midBandCompressorAttack,
	midBandCompressorRelease,
	midBandCompressorRatio,
	// -- High Band Compressor
	highBandCompressorBypass,
	highBandCompressorThreshold,
	highBandCompressorAttack,
	highBandCompressorRelease,
	highBandCompressorRatio,

	// -- Phaser
	phaserBypass,
	phaserRate,
	phaserDepth,
	phaserCentreFrequency,
	phaserFeedback,
	phaserMix,

	//==============================================================================
	countParams // value to keep track of the total number of parameters
};