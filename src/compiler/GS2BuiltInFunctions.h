#pragma once

#ifndef GS2BUILTINFUNCTIONS_H
#define GS2BUILTINFUNCTIONS_H

#include <string_view>
#include <cstdint>
#include "opcodes.h"

enum CmdFlags
{
	CMD_NOOPT			= uint8_t(0),			// No options set
	CMD_USE_ARRAY		= uint8_t(1 << 0),		// Pass arguments to function call in an array
	CMD_REVERSE_ARGS	= uint8_t(1 << 1),		// Reverse the order in which arguments are visited (default)
	CMD_RETURN_VALUE	= uint8_t(1 << 2),		// Call returns a value (needed to discard unused return values)
	CMD_OBJECT_FIRST	= uint8_t(1 << 3)		// Visit the object before you visit arguments (needed for setarray)
};

constexpr uint8_t DEFAULT_CMD_FLAGS = CMD_USE_ARRAY | CMD_REVERSE_ARGS | CMD_RETURN_VALUE;
constexpr uint8_t DEFAULT_OBJ_CMD_FLAGS = CMD_USE_ARRAY | CMD_REVERSE_ARGS | CMD_RETURN_VALUE;

struct BuiltInCmd
{
	std::string_view name;										// Function Name
	opcode::Opcode op;										// Op-code for built in command, or OP_CALL
	opcode::Opcode convert_object_op{ opcode::OP_NONE };			// Convert object to this type [used for object.call() functions]
	uint8_t flags = (CMD_REVERSE_ARGS | CMD_RETURN_VALUE);	// See above for cmd options
	std::string_view sig;
};

constexpr BuiltInCmd defaultCall = {
	"",
	opcode::OP_CALL,
	opcode::OP_NONE,
	DEFAULT_CMD_FLAGS
};

constexpr BuiltInCmd defaultObjCall = {
	"",
	opcode::OP_CALL,
	opcode::OP_CONV_TO_OBJECT,
	DEFAULT_OBJ_CMD_FLAGS
};

const BuiltInCmd* findBuiltInCmd(std::string_view name);
const BuiltInCmd* findBuiltInObjCmd(std::string_view name);

#endif
