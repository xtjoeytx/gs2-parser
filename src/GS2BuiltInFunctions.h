#pragma once

#ifndef GS2BUILTINFUNCTIONS_H
#define GS2BUILTINFUNCTIONS_H

#include <string>
#include <unordered_map>
#include "opcodes.h"

struct BuiltInCmd
{
	std::string name;								// Function Name
	opcode::Opcode op;								// Op-code for built in command, or OP_CALL
	bool useArray;									// Parameters should be emitted as an array
	opcode::Opcode convert_op{ opcode::OP_NONE };	// Convert result to this type
};

const BuiltInCmd defaultCall = {
	"", opcode::OP_CALL, true
};

const BuiltInCmd defaultObjCall = {
	"", opcode::OP_CALL, true, opcode::OP_CONV_TO_OBJECT
};

struct GS2BuiltInFunctions
{
	std::unordered_map<std::string, BuiltInCmd> builtInCmdMap;
	std::unordered_map<std::string, BuiltInCmd> builtInObjMap;

	GS2BuiltInFunctions() { }

	GS2BuiltInFunctions(GS2BuiltInFunctions&& o) noexcept;
	GS2BuiltInFunctions(const GS2BuiltInFunctions&) = delete;

	GS2BuiltInFunctions& operator= (GS2BuiltInFunctions&& o) noexcept;
	GS2BuiltInFunctions& operator= (const GS2BuiltInFunctions&) = delete;

	static GS2BuiltInFunctions getBuiltIn();
};

inline GS2BuiltInFunctions& GS2BuiltInFunctions::operator=(GS2BuiltInFunctions&& o) noexcept
{
	if (this != &o)
	{
		builtInCmdMap = std::move(o.builtInCmdMap);
		builtInObjMap = std::move(o.builtInObjMap);
	}

	return *this;
}

inline GS2BuiltInFunctions::GS2BuiltInFunctions(GS2BuiltInFunctions&& o) noexcept
{
	*this = std::move(o);
}

#endif
