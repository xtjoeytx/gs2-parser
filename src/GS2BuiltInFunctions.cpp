#include "GS2BuiltInFunctions.h"

const BuiltInCmd builtInCmds[] = {
	{"sleep", opcode::OP_SLEEP, false},
	{"makevar", opcode::OP_MAKEVAR, false, opcode::OP_CONV_TO_STRING},
	{"format", opcode::OP_FORMAT, true},
	{"abs", opcode::OP_ABS, false},
	{"random", opcode::OP_RANDOM, false},
	{"sin", opcode::OP_SIN, false},
	{"cos", opcode::OP_COS, false},

	{"arctan", opcode::OP_ARCTAN, false},
	{"exp", opcode::OP_EXP, false},
	{"log", opcode::OP_LOG, false},
	{"min", opcode::OP_MIN, false},
	{"max", opcode::OP_MAX, false},
	{"getangle", opcode::OP_GETANGLE, false},
	{"getdir", opcode::OP_GETDIR, false},
	{"vecx", opcode::OP_VECX, false},
	{"vecy", opcode::OP_VECY, false},
};

const BuiltInCmd builtInObjCmds[] = {
	{"index", opcode::OP_OBJ_INDEX, false},
	{"type", opcode::OP_OBJ_TYPE, false, opcode::OP_CONV_TO_OBJECT},

	//
	{"indices", opcode::OP_OBJ_INDICES, false},
	{"link", opcode::OP_OBJ_LINK, false},
	{"trim", opcode::OP_OBJ_TRIM, false, opcode::OP_CONV_TO_STRING},
	{"length", opcode::OP_OBJ_LENGTH, false, opcode::OP_CONV_TO_STRING},
	{"pos", opcode::OP_OBJ_POS, false},
	{"charat", opcode::OP_OBJ_CHARAT, false},
	{"substring", opcode::OP_OBJ_SUBSTR, false},
	{"starts", opcode::OP_OBJ_STARTS, false},
	{"ends", opcode::OP_OBJ_ENDS, false},
	{"tokenize", opcode::OP_OBJ_TOKENIZE, false},
	{"positions", opcode::OP_OBJ_POSITIONS, false},
	{"size", opcode::OP_OBJ_SIZE, false, opcode::OP_CONV_TO_OBJECT},
	{"subarray", opcode::OP_OBJ_SUBARRAY, false},
	{"clear", opcode::OP_OBJ_CLEAR, false},
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
