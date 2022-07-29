#include "GS2BuiltInFunctions.h"

// Signature characters: [return][...params]
// - -> discard (FOR RETURN VAR) (no conversion)
// x -> NONE (no conversion)
// f -> OP_CONV_TO_FLOAT
// o -> OP_CONV_TO_OBJECT
// s -> OP_CONV_TO_STRING

const BuiltInCmd builtInCmds[] = {
	{
		.name = "sleep",
		.op = opcode::OP_SLEEP,
		.flags = CMD_NOOPT,
		.sig = "-f",
	},

	{
		.name = "sin",
		.op = opcode::OP_SIN,
		.sig = "ff"
	},

	{
		.name = "cos",
		.op = opcode::OP_COS,
		.sig = "ff"
	},

	{
		.name = "arctan",
		.op = opcode::OP_ARCTAN,
		.sig = "ff"
	},

	{
		.name = "vecx",
		.op = opcode::OP_VECX,
		.sig = "ff"
	},

	{
		.name = "vecy",
		.op = opcode::OP_VECY,
		.sig = "ff"
	},

	{
		.name = "abs",
		.op = opcode::OP_ABS,
		.sig = "ff"
	},

	{
		.name = "exp",
		.op = opcode::OP_EXP,
		.sig = "ff"
	},

	{
		.name = "log",
		.op = opcode::OP_LOG,
		.sig = "ff"
	},

	{
		.name = "min",
		.op = opcode::OP_MIN,
		.sig = "fff"
	},

	{
		.name = "max",
		.op = opcode::OP_MAX,
		.sig = "fff"
	},

    {
        .name = "pow",
        .op = opcode::OP_POW,
        .flags = CMD_RETURN_VALUE,
        .sig = "fff"
    },

	{
		.name = "random",
		.op = opcode::OP_RANDOM,
		.sig = "fff"
	},

	{
		.name = "setarray",
		.op = opcode::OP_SETARRAY,
		.flags = CmdFlags::CMD_NOOPT,
		.sig = "-of"
	},

	{
		.name = "getangle",
		.op = opcode::OP_GETANGLE,
		.sig = "fff"
	},

	{
		.name = "getdir",
		.op = opcode::OP_GETDIR,
		.sig = "fff"
	},

	{
		.name = "format",
		.op = opcode::OP_FORMAT,
		.flags = CMD_USE_ARRAY | CMD_REVERSE_ARGS | CMD_RETURN_VALUE,
		.sig = "xs"
	},

	{
		.name = "makevar",
		.op = opcode::OP_MAKEVAR,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.sig = "s"
	}
};

const BuiltInCmd builtInObjCmds[] = {
	{
		.name = "index",
		.op = opcode::OP_OBJ_INDEX
	},

	{
		.name = "type",
		.op = opcode::OP_OBJ_TYPE,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT
	},

	{
		.name = "indices",
		.op = opcode::OP_OBJ_INDICES
	},

	{
		.name = "link",
		.op = opcode::OP_OBJ_LINK
	},

	{
		.name = "trim",
		.op = opcode::OP_OBJ_TRIM,
		.convert_object_op = opcode::OP_CONV_TO_STRING
	},

	{
		.name = "length",
		.op = opcode::OP_OBJ_LENGTH,
		.convert_object_op = opcode::OP_CONV_TO_STRING
	},

	{
		.name = "pos",
		.op = opcode::OP_OBJ_POS,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
	},

	{
		.name = "charat",
		.op = opcode::OP_OBJ_CHARAT,
		//.convert_object_op = opcode::OP_CONV_TO_STRING
	},

	{
		.name = "substring",
		.op = opcode::OP_OBJ_SUBSTR,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
	},

	{
		.name = "starts",
		.op = opcode::OP_OBJ_STARTS,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
	},

	{
		.name = "ends",
		.op = opcode::OP_OBJ_ENDS,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
	},

	{
		.name = "tokenize",
		.op = opcode::OP_OBJ_TOKENIZE,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
	},

	{
		.name = "positions",
		.op = opcode::OP_OBJ_POSITIONS,
	},

	{
		.name = "size",
		.op = opcode::OP_OBJ_SIZE,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT
	},
	{
		.name = "subarray",
		.op = opcode::OP_OBJ_SUBARRAY
	},
	{
		.name = "clear",
		.op = opcode::OP_OBJ_CLEAR,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT,
		.flags = CmdFlags::CMD_NOOPT,
	},
	{
		.name = "add",
		.op = opcode::OP_OBJ_ADDSTRING,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT,
		.flags = CmdFlags::CMD_OBJECT_FIRST,
	},
};

GS2BuiltInFunctions GS2BuiltInFunctions::getBuiltIn()
{
	GS2BuiltInFunctions functions;

	for (const auto& cmd : builtInCmds)
		functions.builtInCmdMap.insert({ cmd.name, cmd });

	for (const auto& cmd : builtInObjCmds)
		functions.builtInObjMap.insert({ cmd.name, cmd });

	return functions;
}
