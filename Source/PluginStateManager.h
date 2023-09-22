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
	std::array<ParameterDefinition, ControlID::countParams> parameterDefinitions = createParameterDefinitions();

	std::array<ParameterDefinition, ControlID::countParams> createParameterDefinitions();
};
