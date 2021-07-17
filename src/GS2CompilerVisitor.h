#pragma once

#ifndef GS2COMPILER_H
#define GS2COMPILER_H

#include <stack>
#include "astvisitor.h"
#include "GS2Bytecode.h"

class ParserData;

struct LoopBreakPoint
{
    size_t continuepoint;
    std::vector<size_t> breakPointLocs;
};

class GS2CompilerVisitor : public NodeVisitor
{
    GS2Bytecode byteCode;
    ParserData *parserData;

    public:
        GS2CompilerVisitor(ParserData *data) : NodeVisitor(), parserData(data) { }
        virtual ~GS2CompilerVisitor() = default;

        void Reset() {
            byteCode.Reset();
        }

        Buffer getByteCode() {
            return byteCode.getByteCode();
        }

        virtual void Visit(Node *node);
        virtual void Visit(StatementNode *node);
        virtual void Visit(StatementBlock *node);
        virtual void Visit(StatementIfNode *node);
        virtual void Visit(StatementFnDeclNode *node);
        virtual void Visit(StatementNewNode *node);
        virtual void Visit(StatementBreakNode *node);
        virtual void Visit(StatementContinueNode *node);
        virtual void Visit(StatementReturnNode *node);
        virtual void Visit(StatementForNode *node);
        virtual void Visit(StatementForEachNode *node);
        virtual void Visit(StatementSwitchNode *node);
        virtual void Visit(StatementWhileNode *node);
        virtual void Visit(StatementWithNode *node);
        virtual void Visit(ExpressionNode *node);
        virtual void Visit(ExpressionIdentifierNode *node);
        virtual void Visit(ExpressionStringConstNode *node);
        virtual void Visit(ExpressionIntegerNode *node);
        virtual void Visit(ExpressionNumberNode *node);
        virtual void Visit(ExpressionCastNode *node);
        virtual void Visit(ExpressionFnCallNode *node);
        virtual void Visit(ExpressionNewNode *node);
        virtual void Visit(ExpressionBinaryOpNode *node);
        virtual void Visit(ExpressionUnaryOpNode *node);
        virtual void Visit(ExpressionObjectAccessNode *node);
        virtual void Visit(ExpressionListNode *node);

        std::stack<LoopBreakPoint> breakPoints;
};

/*
class GS2LoopIdentDiscoveryVisitor : public NodeVisitor
{
    ParserData *parserData;

public:
    GS2LoopIdentDiscoveryVisitor(ParserData* data) : NodeVisitor(), parserData(data) { }
    virtual ~GS2LoopIdentDiscoveryVisitor() = default;

    virtual void Visit(Node* node) {}
    virtual void Visit(StatementNode* node) {}
    virtual void Visit(StatementBlock* node) {
        for (const auto& n : node->statements)
            n->visit(this);
    }

    virtual void Visit(StatementIfNode* node);
    virtual void Visit(StatementFnDeclNode* node);
    virtual void Visit(StatementNewNode* node);
    virtual void Visit(StatementBreakNode* node);
    virtual void Visit(StatementContinueNode* node);
    virtual void Visit(StatementReturnNode* node);
    virtual void Visit(StatementForNode* node);
    virtual void Visit(StatementForEachNode* node);
    virtual void Visit(StatementSwitchNode* node);
    virtual void Visit(StatementWhileNode* node);
    virtual void Visit(StatementWithNode* node);
    virtual void Visit(ExpressionNode* node);
    virtual void Visit(ExpressionIdentifierNode* node);
    virtual void Visit(ExpressionStringConstNode* node);
    virtual void Visit(ExpressionIntegerNode* node);
    virtual void Visit(ExpressionNumberNode* node);
    virtual void Visit(ExpressionCastNode* node);
    virtual void Visit(ExpressionFnCallNode* node);
    virtual void Visit(ExpressionNewNode* node);
    virtual void Visit(ExpressionBinaryOpNode* node);
    virtual void Visit(ExpressionUnaryOpNode* node);
    virtual void Visit(ExpressionObjectAccessNode* node);
    virtual void Visit(ExpressionListNode* node);
};
*/

#endif
