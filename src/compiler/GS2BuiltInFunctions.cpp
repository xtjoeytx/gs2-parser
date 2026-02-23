#include <algorithm>
#include <array>
#include "GS2BuiltInFunctions.h"

// Signature characters: [return][...params]
// - -> discard (FOR RETURN VAR) (no conversion)
// x -> NONE (no conversion)
// f -> OP_CONV_TO_FLOAT
// o -> OP_CONV_TO_OBJECT
// s -> OP_CONV_TO_STRING

template <std::size_t N>
consteval std::array<BuiltInCmd, N> sortByName(std::array<BuiltInCmd, N> arr)
{
	std::ranges::sort(arr, [](const BuiltInCmd& a, const BuiltInCmd& b) {
		return a.name < b.name;
	});
	return arr;
}

static constexpr auto builtInCmds = sortByName(std::array{
	BuiltInCmd{
		.name = "sleep",
		.op = opcode::OP_SLEEP,
		.flags = CMD_NOOPT,
		.sig = "-f",
	},

	BuiltInCmd{
		.name = "sin",
		.op = opcode::OP_SIN,
		.sig = "ff"
	},

	BuiltInCmd{
		.name = "char",
		.op = opcode::OP_CHAR,
		.sig = "ff"
	},

	BuiltInCmd{
		.name = "cos",
		.op = opcode::OP_COS,
		.sig = "ff"
	},

	BuiltInCmd{
		.name = "arctan",
		.op = opcode::OP_ARCTAN,
		.sig = "ff"
	},

	BuiltInCmd{
		.name = "vecx",
		.op = opcode::OP_VECX,
		.sig = "ff"
	},

	BuiltInCmd{
		.name = "vecy",
		.op = opcode::OP_VECY,
		.sig = "ff"
	},

	BuiltInCmd{
		.name = "abs",
		.op = opcode::OP_ABS,
		.sig = "ff"
	},

	BuiltInCmd{
		.name = "exp",
		.op = opcode::OP_EXP,
		.sig = "ff"
	},

	BuiltInCmd{
		.name = "log",
		.op = opcode::OP_LOG,
		.sig = "fff"
	},

	BuiltInCmd{
		.name = "min",
		.op = opcode::OP_MIN,
		.sig = "fff"
	},

	BuiltInCmd{
		.name = "max",
		.op = opcode::OP_MAX,
		.sig = "fff"
	},

    BuiltInCmd{
        .name = "pow",
        .op = opcode::OP_POW,
        .flags = CMD_RETURN_VALUE,
        .sig = "fff"
    },

	BuiltInCmd{
		.name = "random",
		.op = opcode::OP_RANDOM,
		.sig = "fff"
	},

	BuiltInCmd{
		.name = "arraylen",
		.op = opcode::OP_OBJ_SIZE,
		.sig = "fo"
	},

	BuiltInCmd{
		.name = "sarraylen",
		.op = opcode::OP_OBJ_SIZE,
		.sig = "fo"
	},

	BuiltInCmd{
		.name = "setarray",
		.op = opcode::OP_SETARRAY,
		.flags = CmdFlags::CMD_NOOPT,
		.sig = "-of"
	},

	BuiltInCmd{
		.name = "getangle",
		.op = opcode::OP_GETANGLE,
		.flags = CMD_RETURN_VALUE,
		.sig = "fff"
	},

	BuiltInCmd{
		.name = "getdir",
		.op = opcode::OP_GETDIR,
		.flags = CMD_RETURN_VALUE,
		.sig = "fff"
	},

    BuiltInCmd{
        .name = "waitfor",
        .op = opcode::OP_WAITFOR,
        .flags = CMD_RETURN_VALUE,
        .sig = "xssf"
    },

	BuiltInCmd{
		.name = "format",
		.op = opcode::OP_FORMAT,
		.flags = CMD_USE_ARRAY | CMD_REVERSE_ARGS | CMD_RETURN_VALUE,
		.sig = "xs"
	},

	BuiltInCmd{
		.name = "makevar",
		.op = opcode::OP_MAKEVAR,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.sig = "s"
	}
});

static constexpr auto builtInObjCmds = sortByName(std::array{
	BuiltInCmd{
		.name = "index",
		.op = opcode::OP_OBJ_INDEX,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
		.sig = "fx"
	},
	BuiltInCmd{
		.name = "type",
		.op = opcode::OP_OBJ_TYPE,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT
	},
	BuiltInCmd{
		.name = "indices",
		.op = opcode::OP_OBJ_INDICES
	},
	BuiltInCmd{
		.name = "link",
		.op = opcode::OP_OBJ_LINK
	},
	BuiltInCmd{
		.name = "trim",
		.op = opcode::OP_OBJ_TRIM,
		.convert_object_op = opcode::OP_CONV_TO_STRING
	},
	BuiltInCmd{
		.name = "length",
		.op = opcode::OP_OBJ_LENGTH,
		.convert_object_op = opcode::OP_CONV_TO_STRING
	},
	BuiltInCmd{
		.name = "pos",
		.op = opcode::OP_OBJ_POS,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
		.sig = "fs"
	},
	BuiltInCmd{
		.name = "charat",
		.op = opcode::OP_OBJ_CHARAT,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
		.sig = "sf"
	},
	BuiltInCmd{
		.name = "substring",
		.op = opcode::OP_OBJ_SUBSTR,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
		.sig = "sff"
	},
	BuiltInCmd{
		.name = "starts",
		.op = opcode::OP_OBJ_STARTS,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
	},
	BuiltInCmd{
		.name = "ends",
		.op = opcode::OP_OBJ_ENDS,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
	},
	BuiltInCmd{
		.name = "tokenize",
		.op = opcode::OP_OBJ_TOKENIZE,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
	},
	BuiltInCmd{
		.name = "positions",
		.op = opcode::OP_OBJ_POSITIONS,
		.convert_object_op = opcode::OP_CONV_TO_STRING,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_RETURN_VALUE,
		.sig = "os"
	},
	BuiltInCmd{
		.name = "size",
		.op = opcode::OP_OBJ_SIZE,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT
	},
	BuiltInCmd{
		.name = "subarray",
		.op = opcode::OP_OBJ_SUBARRAY
	},
	BuiltInCmd{
		.name = "clear",
		.op = opcode::OP_OBJ_CLEAR,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT,
		.flags = CmdFlags::CMD_NOOPT,
	},
	BuiltInCmd{
		.name = "add",
		.op = opcode::OP_OBJ_ADDSTRING,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT,
		.flags = CmdFlags::CMD_OBJECT_FIRST,
	},
	BuiltInCmd{
		.name = "delete",
		.op = opcode::OP_OBJ_DELETESTRING,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT,
		.flags = CmdFlags::CMD_OBJECT_FIRST,
	},
	BuiltInCmd{
		.name = "insert",
		.op = opcode::OP_OBJ_INSERTSTRING,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_REVERSE_ARGS,
	},
	BuiltInCmd{
		.name = "remove",
		.op = opcode::OP_OBJ_REMOVESTRING,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_REVERSE_ARGS,
	},
	BuiltInCmd{
		.name = "replace",
		.op = opcode::OP_OBJ_REPLACESTRING,
		.convert_object_op = opcode::OP_CONV_TO_OBJECT,
		.flags = CmdFlags::CMD_OBJECT_FIRST | CmdFlags::CMD_REVERSE_ARGS,
	},
});

template <std::size_t N>
constexpr const BuiltInCmd* findIn(const std::array<BuiltInCmd, N>& table, std::string_view name)
{
	auto it = std::ranges::lower_bound(table, name, std::less<>{}, &BuiltInCmd::name);

	if (it != table.end() && it->name == name)
		return &*it;

	return nullptr;
}

const BuiltInCmd* findBuiltInCmd(std::string_view name)
{
	return findIn(builtInCmds, name);
}

const BuiltInCmd* findBuiltInObjCmd(std::string_view name)
{
	return findIn(builtInObjCmds, name);
}
