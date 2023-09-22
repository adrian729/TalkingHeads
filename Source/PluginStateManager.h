/*
  ==============================================================================

	This file contains the data structures and objects to define the plugin's parameter structure.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "parameterTypes.h"
#include "ParameterObject.h"
#include "ParameterDefinition.h"

//==============================================================================
class PluginStateManager
{
public:
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	PluginStateManager();
	~PluginStateManager();

	//==============================================================================
	// -- METHODS
	//==============================================================================
	std::array<ParameterDefinition, ControlID::countParams>& getParameterDefinitions();

private:
	//==============================================================================
	// --- Parameters definitions - make sure each item in the array matches the order of the enum
	static std::array<juce::String, ControlID::countParams> paramIDs;
	static std::array<juce::String, ControlID::countParams> paramNames;
	static std::array<ParameterDefinition, ControlID::countParams> parameterDefinitions;

	static std::array<juce::String, ControlID::countParams> createParamIDs();
	static std::array<juce::String, ControlID::countParams> createParamNames();
	static std::array<ParameterDefinition, ControlID::countParams> createParameterDefinitions();
};
