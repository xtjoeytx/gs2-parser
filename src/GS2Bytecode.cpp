#include <cassert>

#include "GS2Bytecode.h"
#include "encoding/graalencoding.h"

 enum
 {
     SEGMENT_GS1FLAGS = 1,
	 SEGMENT_FUNCTIONTABLE = 2,
	 SEGMENT_STRINGTABLE = 3,
	 SEGMENT_BYTECODE = 4
 };

// Format:
// {GINT2(LENGTH_OF_STARTSECTION)}{STARTSECTION}{SEGMENTS}

// Start Section:
// weapon,npcName,saveToDisk[0,1],GINT5(HASH1)GINT5(HASH2)

// Segments: GS1EventFlags:1, FunctionNames:2, Strings:3, Bytecode:4
// {INT4(SEGMENT_TYPE)}{INT4(SEGMENT_LEN)}

// GS1EventFlags: {INT4(0)} - bitflags
// FunctionNames: {INT4(INSTRUCT_ID)}{CSTR-NULL-TERMINATED}...
// StringTable: {CSTR-NULL-TERMINATED}...
// Bytecode: ...

int32_t GS2Bytecode::getStringConst(const std::string& str)
{
	auto it = std::find(stringTable.begin(), stringTable.end(), str);
	if (it == stringTable.end())
	{
		stringTable.push_back(str);
		return int32_t(stringTable.size() - 1);
	}

	auto dist = std::distance(stringTable.begin(), it);
	assert(std::abs(dist) < std::numeric_limits<int32_t>::max());

	return int32_t(dist);
}

Buffer GS2Bytecode::getByteCode()
{
	Buffer byteCode;

	// GS1EventFlags
	{
		Buffer gs1flags;
		gs1flags.Write<encoding::Int32>(0); // bitflag for gs1 events

		byteCode.Write<encoding::Int32>(SEGMENT_GS1FLAGS);
		byteCode.Write<encoding::Int32>(uint32_t(gs1flags.length()));
		byteCode.write(gs1flags);
	}

	// Function Names
	{
		Buffer functionNames;
		for (const auto& func : functionTable)
		{
			functionNames.Write<encoding::Int32>(func.opIndex);
			functionNames.write(func.functionName.c_str(), func.functionName.length());
			functionNames.write('\0');

			// emit a jump before the function declaration to the last op index
			emit(short(opIndex), func.jmpLoc - 2);
		}

		byteCode.Write<encoding::Int32>(SEGMENT_FUNCTIONTABLE);
		byteCode.Write<encoding::Int32>(uint32_t(functionNames.length()));
		byteCode.write(functionNames);
	}

	// String Table
	{
		Buffer stringTableBuffer;
		for (const auto& str : stringTable)
		{
			stringTableBuffer.write(str.c_str(), str.length());
			stringTableBuffer.write('\0');
		}

		byteCode.Write<encoding::Int32>(SEGMENT_STRINGTABLE);
		byteCode.Write<encoding::Int32>(uint32_t(stringTableBuffer.length()));
		byteCode.write(stringTableBuffer);
	}

	// Bytecode
	byteCode.Write<encoding::Int32>(SEGMENT_BYTECODE);
	byteCode.Write<encoding::Int32>(uint32_t(bytecode.length()));
	byteCode.write(bytecode);
	byteCode.write('\n');

	return byteCode;
}

void GS2Bytecode::addFunction(const std::string& functionName, uint32_t opIdx, size_t jmpLoc)
{
	auto it = std::find_if(functionTable.begin(), functionTable.end(), [functionName](const FunctionEntry& e) {
		return (functionName == e.functionName);
	});

	if (it == functionTable.end())
	{
		functionTable.push_back(FunctionEntry{
			functionName,
			opIdx,
			jmpLoc
		});
	}
	else
	{
#ifdef DBGEMITTERS
		printf("Already added function %s\n", functionName.c_str());
#endif
	}
}

void GS2Bytecode::emit(opcode::Opcode op)
{
#ifdef DBGEMITTERS
	printf("%5zu EMIT OPER: %s (%d) loc: %d\n", bytecode.length(), opcode::OpcodeToString(op).c_str(), op, opIndex);
#endif

	bytecode.write((char)op);
	lastOp = op;
	++opIndex;
}

void GS2Bytecode::emit(char v, size_t pos)
{
#ifdef DBGEMITTERS
	printf("%5zu EMIT byte: %d\n", (pos == SIZE_MAX ? bytecode.length() : pos), (uint8_t)v);
#endif

	if (pos != SIZE_MAX)
	{
		auto current = bytecode.length();
		bytecode.setWritePos(pos);
		bytecode.write(v);
		bytecode.setWritePos(current);
	}
	else
	{
		bytecode.write(v);
	}
}

void GS2Bytecode::emit(short v, size_t pos)
{
#ifdef DBGEMITTERS
	printf("%5zu EMIT short: %d\n", (pos == SIZE_MAX ? bytecode.length() : pos), v);
#endif

	if (pos != SIZE_MAX)
	{
		bytecode.Write<encoding::Int16>(v, pos);
	}
	else
	{
		bytecode.Write<encoding::Int16>(v);
	}
}

void GS2Bytecode::emit(int v, size_t pos)
{
#ifdef DBGEMITTERS
	printf("%5zu EMIT int: %d\n", (pos == SIZE_MAX ? bytecode.length() : pos), v);
#endif

	if (pos != SIZE_MAX)
	{
		bytecode.Write<encoding::Int32>(v, pos);
	}
	else
	{
		bytecode.Write<encoding::Int32>(v);
	}
 }

void GS2Bytecode::emit(const std::string& v)
{
#ifdef DBGEMITTERS
	printf("%5zu EMIT null-terminated str: %s (len: %zu)\n", bytecode.length(), v.c_str(), v.length()+1);
#endif

	bytecode.write(v.c_str(), v.length());
	bytecode.write('\0');
}

void GS2Bytecode::emitConversionOp(ExpressionType typeSrc, ExpressionType typeDst)
{
	if (typeSrc != typeDst)
	{
		if (typeDst == ExpressionType::EXPR_NUMBER)
		{
			if (typeSrc != ExpressionType::EXPR_INTEGER)
				emit(opcode::Opcode::OP_CONV_TO_FLOAT);
		}
		else if (typeDst == ExpressionType::EXPR_STRING)
		{
			emit(opcode::Opcode::OP_CONV_TO_STRING);
		}
		else if (typeDst == ExpressionType::EXPR_OBJECT)
		{
			emit(opcode::Opcode::OP_CONV_TO_OBJECT);
		}
	}
}

void GS2Bytecode::emitDynamicNumber(int32_t val)
{
	assert(val == std::abs(val));

	// Strings use 0xF0 -> 0xF2, numbers use 0xF3 -> 0xF5
	// 0xF6 is used for null-terminated strings converted to doubles
	char offset = 0;

	switch (getLastOp())
	{
		case opcode::OP_SET_INDEX:
		case opcode::OP_SET_INDEX_TRUE:
		case opcode::OP_TYPE_NUMBER:
			offset = 3;
			break;

		case opcode::OP_TYPE_VAR:
		case opcode::OP_TYPE_STRING:
			//offset = 0;
			break;

		default:
			printf("Previous opcode should be a number or string!!\n");
			return;
	}

	if (val <= std::numeric_limits<int8_t>::max())
	{
		emit(char(0xF0 + offset));
		emit(char(val));
	}
	else if (val <= std::numeric_limits<int16_t>::max())
	{
		emit(char(0xF1 + offset));
		emit(short(val));
	}
	else if (val <= std::numeric_limits<int32_t>::max())
	{
		emit(char(0xF2 + offset));
		emit(int(val));
	}
}

void GS2Bytecode::emitDoubleNumber(const std::string& num)
{
	assert(getLastOp() == opcode::OP_TYPE_NUMBER);

	emit(char(0xF6));
	emit(num);
}
