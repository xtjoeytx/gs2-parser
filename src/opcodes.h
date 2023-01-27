#pragma once

#ifndef OPCODES_H
#define OPCODES_H

#include <string>

namespace opcode
{
	// Potential unknown standard functions:
	// char(ascii), ex: char(65) -> 'A'
	// waitfor(objectname,event[,timeout])

	enum Opcode
	{
		// @formatter:off

		/*  NAME            SEMANTIC                        */
		/* ------------------------------------------------ */
		OP_NONE = 0,
		OP_SET_INDEX = 1,	//  S(1) =			// likely JMP to opIndex
		OP_SET_INDEX_TRUE = 2,
		OP_OR = 3,
		OP_IF = 4,			// likely JMPIFNOT
		OP_AND = 5,
		OP_CALL = 6,
		OP_RET = 7,			//  Return to location on top of on jump stack
		OP_SLEEP = 8,
		OP_CMD_CALL = 9,	//  may just increase the loop count for the loop limit of 10k
		OP_JMP = 10,		//  JUMP to N(0, 4) by byte offset unconditionally
		OP_UNKNOWN_11 = 11,

		OP_TYPE_NUMBER = 20,
		OP_TYPE_STRING = 21,
		OP_TYPE_VAR = 22,
		OP_TYPE_ARRAY = 23,
		OP_TYPE_TRUE = 24,
		OP_TYPE_FALSE = 25,
		OP_TYPE_NULL = 26,
		OP_PI = 27,

		OP_COPY_LAST_OP = 30,
		OP_SWAP_LAST_OPS = 31,
		OP_INDEX_DEC = 32,
		OP_CONV_TO_FLOAT = 33,
		OP_CONV_TO_STRING = 34,
		OP_MEMBER_ACCESS = 35,
		OP_CONV_TO_OBJECT = 36,
		OP_ARRAY_END	  = 37,
		OP_ARRAY_NEW	  = 38,
		OP_SETARRAY		  = 39,
		OP_INLINE_NEW 	  = 40,
		OP_MAKEVAR		  = 41,
		OP_NEW_OBJECT 	  = 42,
		OP_UNKNOWN_43	  = 43,		// makeOldScriptVar
		OP_INLINE_CONDITIONAL = 44,
		OP_UNKNOWN_45	  = 45,
		OP_UNKNOWN_46	  = 46,
		OP_UNKNOWN_47	  = 47,

		OP_ASSIGN = 50, //  S(1) = S(0)
		OP_FUNC_PARAMS_END = 51,
		OP_INC = 52, //  SET (S(0) = S(0) + 1)
		OP_DEC = 53, //  SET (S(0) = S(0) - 1)
		OP_UNKNOWN_54 = 54,

		OP_ADD = 60, //  PUSH (S(1) + S(0))
		OP_SUB = 61, //  PUSH (S(1) - S(0))
		OP_MUL = 62, //  PUSH (S(1) * S(0))
		OP_DIV = 63, //  PUSH (S(1) / S(0))
		OP_MOD = 64, //  PUSH (S(1) % S(0))
		OP_POW = 65, //  PUSH (pow(S(1), S(0)))
		OP_UNKNOWN_66 = 66,
		OP_UNKNOWN_67 = 67,
		OP_NOT = 68, //  PUSH (!S(0))
		OP_UNARYSUB = 69, //  PUSH (-S(0))
		OP_EQ = 70, //  PUSH (S(1) == S(0))
		OP_NEQ = 71, // PUSH (S(1) != S(0))
		OP_LT = 72, //  PUSH (S(1) < S(0))
		OP_GT = 73, //  PUSH (S(1) > S(0))
		OP_LTE = 74, //  PUSH (S(1) <= S(0))
		OP_GTE = 75, //  PUSH (S(1) >= S(0))
		OP_BWO = 76, //  PUSH (S(1) | S(0))
		OP_BWA = 77, //  PUSH (S(1) & S(0))
		OP_BWX = 78, //  PUSH (S(1) ^ S(0))
		OP_BWI = 79, //  PUSH (~S(0))
		OP_IN_RANGE = 80,
		OP_IN_OBJ = 81,
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
		OP_BW_LEFTSHIFT = 101,	//  PUSH (S(1) << S(0))
		OP_BW_RIGHTSHIFT = 102,	//  PUSH (S(1) >> S(0))
		OP_CHAR = 103,
		OP_OBJ_COMPARE = 104,	// something like that

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
		OP_ARRAY_ASSIGN = 132,
		OP_ARRAY_MULTIDIM = 133,
		OP_ARRAY_MULTIDIM_ASSIGN = 134,
		OP_OBJ_SUBARRAY = 135,
		OP_OBJ_ADDSTRING = 136,
		OP_OBJ_DELETESTRING = 137,
		OP_OBJ_REMOVESTRING = 138,
		OP_OBJ_REPLACESTRING = 139,
		OP_OBJ_INSERTSTRING = 140,
		OP_OBJ_CLEAR = 141,
		OP_ARRAY_NEW_MULTIDIM = 142,
		OP_WITH = 150,
		OP_WITHEND = 151,
		OP_FOREACH = 163,
		OP_THIS = 180,
		OP_THISO = 181,
		OP_PLAYER = 182,
		OP_PLAYERO = 183,
		OP_LEVEL = 184,
		OP_TEMP = 189,
		OP_PARAMS = 190,
		OP_NUM_OPS //  This is to get the number of operations

		// @formatter:on
	};

	inline bool IsBooleanReturningOp(Opcode opcode)
	{
		switch (opcode)
		{
		case OP_NOT:
		case OP_EQ:
		case OP_NEQ:
		case OP_LT:
		case OP_GT:
		case OP_LTE:
		case OP_GTE:
		case OP_IN_RANGE:
		case OP_IN_OBJ:
			return true;

		default:
			return false;
		}
	}

	inline bool IsReservedIdentOp(Opcode opcode)
	{
		switch (opcode)
		{
		case OP_THIS:
		case OP_THISO:
		case OP_PLAYER:
		case OP_PLAYERO:
		case OP_LEVEL:
		case OP_TEMP:
			return true;

		default:
			return false;
		}
	}

	inline bool IsObjectReturningOp(Opcode opcode)
	{
		switch (opcode)
		{
			case OP_THIS:
			case OP_THISO:
			case OP_PLAYER:
			case OP_PLAYERO:
			case OP_LEVEL:
			case OP_TEMP:
				return true;

			default:
				return false;
		}
	}

	inline std::string OpcodeToString(Opcode opcode)
	{
		switch (opcode)
		{
			case OP_NONE:
				return "OP_NONE";

			case OP_ASSIGN:
				return "OP_ASSIGN";

			case OP_SET_INDEX:
				return "OP_SET_INDEX";

			case OP_SET_INDEX_TRUE:
				return "OP_SET_INDEX_TRUE";

			case OP_IF:
				return "OP_IF";

			case OP_TYPE_TRUE:
				return "OP_TRUE";

			case OP_TYPE_FALSE:
				return "OP_FALSE";

			case OP_TYPE_NULL:
				return "OP_NULL";

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

			case OP_DEC:
				return "OP_DEC";

			case OP_UNARYSUB:
				return "OP_UNARYSUB";

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

			case OP_ARRAY_END:
				return "OP_ARRAY_END";

			case OP_CONV_TO_FLOAT:
				return "OP_CONV_TO_FLOAT";

			case OP_CONV_TO_STRING:
				return "OP_CONV_TO_STRING";

			case OP_MEMBER_ACCESS:
				return "OP_MEMBER_ACCESS";

			case OP_CONV_TO_OBJECT:
				return "OP_CONV_TO_OBJECT";

			case OP_NEW_OBJECT:
				return "OP_NEW_OBJECT";

			case OP_FUNC_PARAMS_END:
				return "OP_FUNC_PARAMS_END";

			case OP_CALL:
				return "OP_CALL";

			case OP_CMD_CALL:
				return "OP_CMD_CALL";

			case OP_JMP:
				return "OP_JMP";

			case OP_INDEX_DEC:
				return "OP_INDEX_DEC";

			case OP_RET:
				return "OP_RET";

			case OP_EQ:
				return "OP_EQ";

			case OP_NEQ:
				return "OP_NEQ";

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

			case OP_ARRAY:
				return "OP_ARRAY[]";

			case OP_OBJ_CHARAT:
				return "OP_OBJ_CHARAT";

			case OP_OBJ_CLEAR:
				return "OP_OBJ_CLEAR";

			case OP_OBJ_ENDS:
				return "OP_OBJ_ENDS";

			case OP_IN_RANGE:
				return "OP_IN_RANGE";

			case OP_IN_OBJ:
				return "OP_IN_OBJ";

			case OP_OBJ_INDEX:
				return "OP_OBJ_INDEX";

			case OP_OBJ_INDICES:
				return "OP_OBJ_INDICES";

			case OP_OBJ_LENGTH:
				return "OP_OBJ_LENGTH";

			case OP_OBJ_LINK:
				return "OP_OBJ_LINK";

			case OP_OBJ_POS:
				return "OP_OBJ_POS";

			case OP_OBJ_POSITIONS:
				return "OP_OBJ_POSITIONS";

			case OP_OBJ_SIZE:
				return "OP_OBJ_SIZE";

			case OP_OBJ_STARTS:
				return "OP_OBJ_STARTS";

			case OP_OBJ_SUBARRAY:
				return "OP_OBJ_SUBARRAY";

			case OP_OBJ_SUBSTR:
				return "OP_OBJ_SUBSTR";

			case OP_OBJ_TOKENIZE:
				return "OP_OBJ_TOKENIZE";

			case OP_OBJ_TRIM:
				return "OP_OBJ_TRIM";

			case OP_OBJ_TYPE:
				return "OP_OBJ_TYPE";

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
