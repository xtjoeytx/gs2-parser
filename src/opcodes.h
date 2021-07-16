#pragma once

#ifndef OPCODES_H
#define OPCODES_H

#include <string>

namespace opcode
{
	enum Opcode {

		// @formatter:off

		/*  NAME            SEMANTIC                        */
		/* ------------------------------------------------ */
		OP_PUSH = 0, //  PUSH N(0, sizeof(PackedValue))
		OP_SET_INDEX = 1, //  S(1) =			// likely JMP to opIndex
		OP_ARR_GET = 2, //  S(1) =

		OP_OR = 3, //  PUSH (S(1) || S(0))
		OP_IF = 4,
		OP_AND = 5, //  PUSH (S(1) && S(0))
		OP_CALL = 6, //  Pushes # of args, followed by args
		OP_RET = 7, //  Return to location on top of on jump stack

		OP_INCPUSH, //  PUSH (S(0)), SET (S(0) = S(0) + 1)
		OP_DECPUSH, //  PUSH (S(0)), SET (S(0) = S(0) - 1)


		OP_CMD_CALL = 9, //  Pushes # of args, followed by args
		OP_JMP = 10, //  JUMP to N(0, 4) by byte offset unconditionally

		OP_JAL, //  JUMP as per prior semantic and pass last location to jump stack
		OP_TYPE_NUMBER = 20,
		OP_TYPE_STRING = 21,
		OP_TYPE_VAR = 22,
		OP_TYPE_ARRAY = 23,
		OP_TYPE_TRUE = 24,
		OP_TYPE_FALSE = 25,
		OP_TYPE_NULL = 26,
		OP_PI = 27,
		OP_COPY_LAST_OP = 30,
		OP_INDEX_DEC = 32,
		OP_CONV_TO_FLOAT = 33,
		OP_CONV_TO_STRING = 34,
		OP_MEMBER_ACCESS = 35,
		OP_CONV_TO_OBJECT = 36,
		OP_INLINE_NEW = 40,
		OP_MAKEVAR = 41,
		OP_NEW_OBJECT = 42,
		OP_ASSIGN = 50, //  S(1) = S(0)
		OP_FUNC_PARAMS_END = 51,

		OP_INC = 52, //  SET (S(0) = S(0) + 1)
		OP_DEC = 53, //  SET (S(0) = S(0) - 1)

		OP_ADD = 60, //  PUSH (S(1) + S(0))
		OP_SUB = 61, //  PUSH (S(1) - S(0))
		OP_MUL = 62, //  PUSH (S(1) * S(0))
		OP_DIV = 63, //  PUSH (S(1) / S(0))
		OP_MOD = 64, //  PUSH (S(1) % S(0))
		OP_POW = 65, //  PUSH (S(1) ^ S(0))

		OP_NOT = 68, //  PUSH (!S(0))

		OP_EQ = 70, //  PUSH (S(1) == S(0))
		OP_LT = 72, //  PUSH (S(1) < S(0))
		OP_GT = 73, //  PUSH (S(1) > S(0))
		OP_LTE = 74, //  PUSH (S(1) <= S(0))
		OP_GTE = 75, //  PUSH (S(1) >= S(0))

		OP_JEZ, //  JUMP to N(0, 4) if S(0) == 0
		OP_JNZ, //  JUMP to N(0, 4) if S(0) != 0

		OP_STOP, // Stops execution

		OP_DBG_OUT, //  Debug output
		OP_OBJ_INDEX = 82,
		OP_OBJ_TYPE = 83,	// gets the type of the var (float 0, string 1, object 2, array 3)
		OP_FORMAT = 84,
		OP_INT = 85,
		OP_ABS = 86,
		OP_RANDOM = 87,
		OP_SIN = 88,
		OP_COS = 89,
		OP_ARCTAN = 90,
		OP_EXP = 91,
		OP_LOG = 92,
		OP_MIN = 93,
		OP_MAX = 94,
		OP_GETANGLE = 95,
		OP_GETDIR = 96,
		OP_VECX = 97,
		OP_VECY = 98,
		OP_OBJ_INDICES = 99,
		OP_OBJ_LINK = 100,
		OP_OBJ_TRIM = 110,
		OP_OBJ_LENGTH = 111,
		OP_OBJ_POS = 112,
		OP_JOIN = 113,
		OP_OBJ_CHARAT = 114,
		OP_OBJ_SUBSTR = 115,
		OP_OBJ_STARTS = 116,
		OP_OBJ_ENDS = 117,
		OP_OBJ_TOKENIZE = 118,
		OP_TRANSLATE = 119,
		OP_OBJ_POSITIONS = 120, // array of positions of the substring in the string
		OP_OBJ_SIZE = 130,
		OP_ARRAY = 131,
		OP_OBJ_SUBARRAY = 135,
		OP_OBJ_CLEAR = 141,
		OP_WITH = 150,
		OP_WITHEND = 151,
		OP_THIS = 180,
		OP_THISO = 181,
		OP_PLAYER = 182,
		OP_PLAYERO = 183,
		OP_LEVEL = 184,
		OP_TEMP = 189,
		OP_NUM_OPS //  This is to get the number of operations

		// @formatter:on
	};

	//inline ExpressionOp OpcodeToString(const std::string& token) {
		
	//}

	inline std::string OpcodeToString(Opcode opcode) {
		switch ( opcode ) {
			case OP_PUSH:
				return "OP_PUSH";

			case OP_ASSIGN:
				return "OP_ASSIGN";

			case OP_SET_INDEX:
				return "OP_SET_INDEX";

			case OP_ARR_GET:
				return "OP_ARR_GET";

			case OP_IF:
				return "OP_IF";

			case OP_ADD:
				return "OP_ADD";

			case OP_SUB:
				return "OP_SUB";

			case OP_MUL:
				return "OP_MUL";

			case OP_DIV:
				return "OP_DIV";

			case OP_MOD:
				return "OP_MOD";

			case OP_POW:
				return "OP_POW";

			case OP_INC:
				return "OP_INC";

			case OP_INCPUSH:
				return "OP_INCPUSH";

			case OP_DEC:
				return "OP_DEC";

			case OP_TYPE_NUMBER:
				return "OP_TYPE_NUMBER";

			case OP_FORMAT:
				return "OP_FORMAT";

			case OP_TYPE_STRING:
				return "OP_TYPE_STRING";

			case OP_TYPE_VAR:
				return "OP_TYPE_VAR";

			case OP_TYPE_ARRAY:
				return "OP_TYPE_ARRAY";

			case OP_CONV_TO_FLOAT:
				return "OP_CONV_TO_FLOAT";

			case OP_CONV_TO_STRING:
				return "OP_CONV_TO_STRING";

			case OP_MEMBER_ACCESS:
				return "OP_MEMBER_ACCESS";

			case OP_CONV_TO_OBJECT:
				return "OP_CONV_TO_OBJECT";

	//		case OP_DECPUSH:
	//			return "OP_DECPUSH";

			case OP_FUNC_PARAMS_END:
				return "OP_FUNC_PARAMS_END";

			case OP_CALL:
				return "OP_CALL";

			case OP_CMD_CALL:
				return "OP_CMD_CALL";

			case OP_JMP:
				return "OP_JMP";

			case OP_JAL:
				return "OP_JAL";

			case OP_INDEX_DEC:
				return "OP_INDEX_DEC";

			case OP_RET:
				return "OP_RET";

			case OP_EQ:
				return "OP_EQ";

			case OP_LT:
				return "OP_LT";

			case OP_GT:
				return "OP_GT";

			case OP_LTE:
				return "OP_LTE";

			case OP_GTE:
				return "OP_GTE";

			case OP_NOT:
				return "OP_NOT";

			case OP_AND:
				return "OP_AND";

			case OP_OR:
				return "OP_OR";

			case OP_JEZ:
				return "OP_JEZ";

			case OP_JNZ:
				return "OP_JNZ";

			case OP_STOP:
				return "OP_STOP";

			case OP_DBG_OUT:
				return "OP_DBG_OUT";

			case OP_JOIN:
				return "OP_JOIN";

			case OP_THIS:
				return "OP_THIS";

			case OP_THISO:
				return "OP_THISO";

			case OP_PLAYER:
				return "OP_PLAYER";

			case OP_PLAYERO:
				return "OP_PLAYERO";

			case OP_LEVEL:
				return "OP_LEVEL";

			case OP_TEMP:
				return "OP_TEMP";

			default:
				return std::string("OP ").append(std::to_string((int)opcode));
		}

		return "Unknown opcode";
	}
}

#endif
