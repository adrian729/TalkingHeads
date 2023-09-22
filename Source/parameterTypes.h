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

enum ParameterType
{
	Bool,
	Choice,
	Float,
	Int
};

enum SmoothingType
{
	NoSmoothing,
	Linear,
	Multiplicative
};

//==============================================================================
// --- CONTROL IDs -- for param definitions array access
//==============================================================================
enum ControlID
{
	// --- stage 0: General -- Bypass ALL // Blend (dry/wet) // Pre Gain
	bypass,
	blend,

	// -- stage 1 -- HPF, LPF, 3 Band EQ
	preGain,
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

	//==============================================================================
	countParams // value to keep track of the total number of parameters
};
//==============================================================================
// --- PARAMETER IDs
//==============================================================================
//// --- stage 0: General -- Bypass ALL // Blend (dry/wet) // Pre Gain
//const juce::String BYPASS_ID = "bypass";
//const juce::String BLEND_ID = "blend";
//const juce::String PRE_GAIN_ID = "preGain";
//
//// -- stage 1 -- HPF, LPF, 3 Band EQ
//// -- HPF
//const juce::String HIGH_PASS_BYPASS_ID = "highpassBypass";
//const juce::String HIGH_PASS_FREQ_ID = "highpassFreq";
//const juce::String HIGH_PASS_SLOPE_ID = "highpassSlope";
//// -- LPF
//const juce::String LOW_PASS_BYPASS_ID = "lowpassBypass";
//const juce::String LOW_PASS_FREQ_ID = "lowpassFreq";
//const juce::String LOW_PASS_SLOPE_ID = "lowpassSlope";
//// -- Band Filter 1
//const juce::String BAND_FILTER_1_BYPASS_ID = "bandFilter1Bypass";
//const juce::String BAND_FILTER_1_PEAK_FREQ_ID = "bandFilter1PeakFreq";
//const juce::String BAND_FILTER_1_PEAK_GAIN_ID = "bandFilter1PeakGain";
//const juce::String BAND_FILTER_1_PEAK_Q_ID = "bandFilter1PeakQ";
//// -- Band Filter 2
//const juce::String BAND_FILTER_2_BYPASS_ID = "bandFilter2Bypass";
//const juce::String BAND_FILTER_2_PEAK_FREQ_ID = "bandFilter2PeakFreq";
//const juce::String BAND_FILTER_2_PEAK_GAIN_ID = "bandFilter2PeakGain";
//const juce::String BAND_FILTER_2_PEAK_Q_ID = "bandFilter2PeakQ";
//// -- Band Filter 3
//const juce::String BAND_FILTER_3_BYPASS_ID = "bandFilter3Bypass";
//const juce::String BAND_FILTER_3_PEAK_FREQ_ID = "bandFilter3PeakFreq";
//const juce::String BAND_FILTER_3_PEAK_GAIN_ID = "bandFilter3PeakGain";
//const juce::String BAND_FILTER_3_PEAK_Q_ID = "bandFilter3PeakQ";
//
//// -- stage 2 -- 3 Band Compressor


//==============================================================================
inline const std::map<ControlID, juce::String>& mapParamIDs()
{
	static std::map<ControlID, juce::String> paramIDs =
	{
		// --- stage 0: General -- Bypass ALL // Blend (dry/wet) // Pre Gain
		{bypass, "bypass"},
		{blend, "blend"},
		{preGain, "preGain"},
		// -- stage 1 -- HPF, LPF, 3 Band EQ
		// -- HPF
		{highpassBypass, "highpassBypass"},
		{highpassFreq, "highpassFreq"},
		{highpassSlope, "highpassSlope"},
		// -- LPF
		{lowpassBypass, "lowpassBypass"},
		{lowpassFreq, "lowpassFreq"},
		{lowpassSlope, "lowpassSlope"},
		// -- Band Filter 1
		{bandFilter1Bypass, "bandFilter1Bypass"},
		{bandFilter1PeakFreq, "bandFilter1PeakFreq"},
		{bandFilter1PeakGain, "bandFilter1PeakGain"},
		{bandFilter1PeakQ, "bandFilter1PeakQ"},
		// -- Band Filter 2
		{bandFilter2Bypass, "bandFilter2Bypass"},
		{bandFilter2PeakFreq, "bandFilter2PeakFreq"},
		{bandFilter2PeakGain, "bandFilter2PeakGain"},
		{bandFilter2PeakQ, "bandFilter2PeakQ"},
		// -- Band Filter 3
		{bandFilter3Bypass, "bandFilter3Bypass"},
		{bandFilter3PeakFreq, "bandFilter3PeakFreq"},
		{bandFilter3PeakGain, "bandFilter3PeakGain"},
		{bandFilter3PeakQ, "bandFilter3PeakQ"},
		// -- stage 2 -- 3 Band Compressor

	};

	return paramIDs;
}

static juce::String getParamID(ControlID controlId)
{
	return mapParamIDs().at(controlId);
}