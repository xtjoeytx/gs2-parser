#include "GS2BuiltInFunctions.h"

const BuiltInCmd builtInCmds[] = {
	{"sleep", opcode::OP_SLEEP },
	{"setarray", opcode::OP_SETARRAY, opcode::OP_NONE, CMD_NOOPT},
	{"makevar", opcode::OP_MAKEVAR, opcode::OP_CONV_TO_STRING },
	{"format", opcode::OP_FORMAT, opcode::OP_NONE, (CMD_USE_ARRAY | CMD_REVERSE_ARGS | CMD_RETURN_VALUE) },
	{"abs", opcode::OP_ABS },
	{"random", opcode::OP_RANDOM },
	{"sin", opcode::OP_SIN },
	{"cos", opcode::OP_COS },

	{"arctan", opcode::OP_ARCTAN },
	{"exp", opcode::OP_EXP },
	{"log", opcode::OP_LOG },
	{"min", opcode::OP_MIN },
	{"max", opcode::OP_MAX },
	{"getangle", opcode::OP_GETANGLE },
	{"getdir", opcode::OP_GETDIR },
	{"vecx", opcode::OP_VECX },
	{"vecy", opcode::OP_VECY },
};

const BuiltInCmd builtInObjCmds[] = {
	{"index", opcode::OP_OBJ_INDEX},
	{"type", opcode::OP_OBJ_TYPE, opcode::OP_CONV_TO_OBJECT},

	//
	{"indices", opcode::OP_OBJ_INDICES },
	{"link", opcode::OP_OBJ_LINK },
	{"trim", opcode::OP_OBJ_TRIM, opcode::OP_CONV_TO_STRING},
	{"length", opcode::OP_OBJ_LENGTH, opcode::OP_CONV_TO_STRING},
	{"pos", opcode::OP_OBJ_POS },
	{"charat", opcode::OP_OBJ_CHARAT},
	{"substring", opcode::OP_OBJ_SUBSTR },
	{"starts", opcode::OP_OBJ_STARTS, opcode::OP_CONV_TO_STRING, CMD_RETURN_VALUE | CMD_OBJECT_FIRST},
	{"ends", opcode::OP_OBJ_ENDS, opcode::OP_CONV_TO_STRING, CMD_RETURN_VALUE | CMD_OBJECT_FIRST },
	{"tokenize", opcode::OP_OBJ_TOKENIZE, opcode::OP_CONV_TO_STRING, CMD_RETURN_VALUE | CMD_OBJECT_FIRST },
	{"positions", opcode::OP_OBJ_POSITIONS },
	{"size", opcode::OP_OBJ_SIZE , opcode::OP_CONV_TO_OBJECT},
	{"subarray", opcode::OP_OBJ_SUBARRAY},
	{"clear", opcode::OP_OBJ_CLEAR, opcode::OP_CONV_TO_OBJECT, CMD_NOOPT},
	{"add", opcode::OP_OBJ_ADDSTRING, opcode::OP_CONV_TO_OBJECT, CMD_OBJECT_FIRST },
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
