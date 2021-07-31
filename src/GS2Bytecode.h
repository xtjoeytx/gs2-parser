#pragma once

#ifndef GS2BYTECODE_H
#define GS2BYTECODE_H

#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include "encoding/buffer.h"
#include "opcodes.h"

struct FunctionEntry
{
    std::string functionName;
    size_t functionIP;
    size_t opIndex;
};

class GS2Bytecode
{
    public:
        GS2Bytecode() : opcodePos(0), opcodeWritePos(0), lastOp(opcode::Opcode::OP_NONE) {}
        
        Buffer getByteCode(const std::string& scriptType, const std::string& scriptName, bool saveToDisk);

        size_t getStringConst(const std::string& str);
        void addFunction(FunctionEntry entry);

        void emit(opcode::Opcode op);
        void emit(char v, size_t pos = SIZE_MAX);
        void emit(short v, size_t pos = SIZE_MAX);
        void emit(int v, size_t pos = SIZE_MAX);
        void emit(const std::string& v);

        void emitDynamicNumber(int32_t val);
        void emitDoubleNumber(const std::string& num);

        opcode::Opcode getLastOp() const;
        size_t getOpcodePos() const;
        size_t getBytecodePos() const;

    private:
        Buffer bytecode;
        std::vector<std::string> stringTable;
        std::vector<FunctionEntry> functionTable;

        opcode::Opcode lastOp;
        size_t opcodePos, opcodeWritePos;
};

inline opcode::Opcode GS2Bytecode::getLastOp() const {
    return lastOp;
}

inline size_t GS2Bytecode::getOpcodePos() const {
    return opcodePos;
}

inline size_t GS2Bytecode::getBytecodePos() const {
    return bytecode.length();
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

#endif
