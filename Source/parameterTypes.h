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
