#include "GS2Bytecode.h"
#include "encoding/graalencoding.h"

// enum
// {
//     GS1EventFlags = 1,
//     FunctionNames = 2,
//     Strings = 3,
//     Bytecode = 4
// };

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

size_t GS2Bytecode::getStringConst(const std::string& str)
{
	auto it = std::find(stringTable.begin(), stringTable.end(), str);
	if (it == stringTable.end()) {
		stringTable.push_back(str);
		return stringTable.size() - 1;
	}

	return std::distance(stringTable.begin(), it);
}

Buffer GS2Bytecode::getByteCode()
{
	Buffer byteCode;

	// Start section
	{
		Buffer startSection;
		startSection.write("weapon,TestCode,1,", strlen("weapon,TestCode,1,"));
		for (int i = 0; i < 10; i++)
			startSection.Write<GraalByte>(0);
		
		byteCode.Write<GraalShort>(startSection.length());
		byteCode.write(startSection);
	}

	// GS1EventFlags
	{
		Buffer gs1flags;
		gs1flags.Write<encoding::Int32>(0); // bitflag for gs1 events
		
		byteCode.Write<encoding::Int32>(1);
		byteCode.Write<encoding::Int32>(gs1flags.length());
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
			
			emit(short(opcodePos), func.functionIP - 2);
		}

		byteCode.Write<encoding::Int32>(2);
		byteCode.Write<encoding::Int32>(functionNames.length());
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

		byteCode.Write<encoding::Int32>(3);
		byteCode.Write<encoding::Int32>(stringTableBuffer.length());
		byteCode.write(stringTableBuffer);
	}

	// Bytecode
	byteCode.Write<encoding::Int32>(4);
	byteCode.Write<encoding::Int32>(bytecode.length());
	byteCode.write(bytecode);
	byteCode.write('\n');

	return byteCode;
}

void GS2Bytecode::addFunction(FunctionEntry entry) {
	auto it = std::find_if(functionTable.begin(), functionTable.end(), [entry](FunctionEntry& e) {
		return (entry.functionName == e.functionName);
	});

	if (it == functionTable.end()) {
		functionTable.push_back(entry);
	}
	else
	{
		printf("Already added function %s\n", entry.functionName.c_str());
	}
}

void GS2Bytecode::emit(opcode::Opcode op)
{
	printf("%5zu EMIT OPER: %s (%d)\n", bytecode.length(), opcode::OpcodeToString(op).c_str(), op);
	
	bytecode.write((char)op);
	lastOp = op;
	++opcodePos;
}

void GS2Bytecode::emit(char v, size_t pos)
{
	printf("%5zu EMIT byte: %d\n", (pos == SIZE_MAX ? bytecode.length() : pos), (uint8_t)v);

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
	printf("%5zu EMIT short: %d\n", (pos == SIZE_MAX ? bytecode.length() : pos), v);

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
	printf("%5zu EMIT int: %d\n", (pos == SIZE_MAX ? bytecode.length() : pos), v);

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
	printf("%5zu EMIT null-terminated str: %s (len: %zu)\n", bytecode.length(), v.c_str(), v.length()+1);

	bytecode.write(v.c_str(), v.length());
	bytecode.write('\0');
}

 void GS2Bytecode::emitDynamicNumber(uint32_t val)
 {
	 // Strings use 0xF0 -> 0xF2, numbers use 0xF3 -> 0xF5
	 // 0xF6 is used for null-terminated strings converted to doubles
	 char offset = 0;
	 if (getLastOp() != opcode::OP_TYPE_STRING)
		offset = 3;

	if (val <= 0xFF)
	{
		emit(char(0xF0 + offset));
		emit(char(val));
	}
	else if (val <= 0xFFFF)
	{
		emit(char(0xF1 + offset));
		emit(short(val));
	}
	else if (val <= 0xFFFFFFFF)
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
