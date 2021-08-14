#pragma once

#ifndef GS2BYTECODE_H
#define GS2BYTECODE_H

#include <cstdint>
#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include "ast/ast.h"
#include "encoding/buffer.h"
#include "opcodes.h"

struct FunctionEntry
{
    std::string functionName;
    uint32_t opIndex;
    size_t jmpLoc;
};

class GS2Bytecode
{
    friend class GS2CompilerVisitor;

    private:
        GS2Bytecode() : opIndex(0), lastOp(opcode::Opcode::OP_NONE) {}
        
        Buffer getByteCode();

        int32_t getStringConst(const std::string& str);
        void addFunction(const std::string& functionName, uint32_t opIdx, size_t jmpLoc);

        void emit(opcode::Opcode op);
        void emit(char v, size_t pos = SIZE_MAX);
        void emit(short v, size_t pos = SIZE_MAX);
        void emit(int v, size_t pos = SIZE_MAX);
        void emit(const std::string& v);
        void emitConversionOp(ExpressionType typeSrc, ExpressionType typeDst);
        void emitDynamicNumber(int32_t val);
        void emitDoubleNumber(const std::string& num);

        /**
         * Gets the last emitted opcode
         *
         * @return
         */
        opcode::Opcode getLastOp() const;

        /**
         * Gets the current operation index. Operation index is incremented
         * after each emit(opcode::Opcode) call
         * @return
         */
        uint32_t getOpIndex() const;

        /**
         * Gets the current length of the bytecode buffer
         *
         * @return
         */
        size_t getBytecodePos() const;

    private:
        Buffer bytecode;
        std::vector<std::string> stringTable;
        std::vector<FunctionEntry> functionTable;

        opcode::Opcode lastOp;
        uint32_t opIndex;
};

inline opcode::Opcode GS2Bytecode::getLastOp() const {
    return lastOp;
}

inline uint32_t GS2Bytecode::getOpIndex() const {
    return opIndex;
}

inline size_t GS2Bytecode::getBytecodePos() const {
    return bytecode.length();
}

#endif
