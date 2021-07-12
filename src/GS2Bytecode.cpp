#include "GS2Bytecode.h"
#include "encoding/graalencoding.h"

size_t GS2Bytecode::getStringConst(const std::string& str)
{
	auto it = std::find(stringTable.begin(), stringTable.end(), str);
	if (it == stringTable.end()) {
		stringTable.push_back(str);
		return stringTable.size() - 1;
	}

	return std::distance(stringTable.begin(), it);
}

// Segments: GS1EventFlags:1, FunctionNames:2, Strings:3, Bytecode:4
// enum
// {
//     GS1EventFlags = 1,
//     FunctionNames = 2,
//     Strings = 3,
//     Bytecode = 4
// };

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

	// Function Names (TODO)
	{
		Buffer functionNames;
		for (const auto& func : functionTable)
		{
			functionNames.Write<encoding::Int32>(func.opIndex);
			functionNames.write(func.functionName.c_str(), func.functionName.length());
			functionNames.write('\0');
			
			//bytecode.write(short(byteCode.getBytecodePos()), jumpIdx);
			emit(short(bytecode.length()), func.functionIP - 2);
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
	printf("%5zu EMIT OPER: %s\n", bytecode.length(), opcode::OpcodeToString(op).c_str());
	
	bytecode.write(op);
	lastOp = op;
	++opcodePos;
}

void GS2Bytecode::emit(char v, size_t pos) {
	printf("%5zu EMIT byte: %d\n", (pos == SIZE_T_MAX ? bytecode.length() : pos), v);

	if (pos != SIZE_T_MAX)
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

void GS2Bytecode::emit(short v, size_t pos) {
	printf("%5zu EMIT short: %d\n", (pos == SIZE_T_MAX ? bytecode.length() : pos), v);

	if (pos != SIZE_T_MAX)
	{
		bytecode.Write<encoding::Int16>(v, pos);
	}
	else
	{
		bytecode.Write<encoding::Int16>(v);
	}
}

void GS2Bytecode::emit(int v, size_t pos) {
	printf("%5zu EMIT int: %d\n", (pos == SIZE_T_MAX ? bytecode.length() : pos), v);

	if (pos != SIZE_T_MAX)
	{
		bytecode.Write<encoding::Int32>(v, pos);
	}
	else
	{
		bytecode.Write<encoding::Int32>(v);
	}
 }

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
