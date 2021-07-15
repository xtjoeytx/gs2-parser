#pragma once

#ifndef GS2COMPILER_H
#define GS2COMPILER_H

#include "astvisitor.h"
#include "GS2Bytecode.h"

class GS2Compiler : public NodeVisitor
{
    GS2Bytecode byteCode;

    public:
        GS2Compiler() : NodeVisitor() { }
        virtual ~GS2Compiler() = default;

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
        virtual void Visit(ExpressionFnCallNode *node);
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
        virtual void Visit(ExpressionCastNode* node);
        virtual void Visit(ExpressionIdentifierNode *node);
        virtual void Visit(ExpressionStringConstNode *node);
        virtual void Visit(ExpressionIntegerNode *node);
        virtual void Visit(ExpressionBinaryOpNode *node);
        virtual void Visit(ExpressionUnaryOpNode *node);
        virtual void Visit(ExpressionObjectAccessNode *node);
        virtual void Visit(ExpressionListNode *node);

        // virtual void Visit(StatementNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementBlock *node) { Visit((Node *)node); }
        // virtual void Visit(StatementIfNode *node) { Visit((Node *)node); }
        // virtual void Visit(ExpressionFnCallNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementFnDeclNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementNewNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementBreakNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementContinueNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementReturnNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementForNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementForEachNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementSwitchNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementWhileNode *node) { Visit((Node *)node); }
        // virtual void Visit(StatementWithNode *node) { Visit((Node *)node); }
        // virtual void Visit(ExpressionNode *node) { Visit((Node *)node); }
        // virtual void Visit(ExpressionIdentifierNode *node) { Visit((Node *)node); }
        // virtual void Visit(ExpressionStringConstNode *node) { Visit((Node *)node); }
        // virtual void Visit(ExpressionIntegerNode *node) { Visit((Node *)node); }
        // virtual void Visit(ExpressionBinaryOpNode *node) { Visit((Node *)node); }
        // virtual void Visit(ExpressionUnaryOpNode *node) { Visit((Node *)node); }
        // virtual void Visit(ExpressionObjectAccessNode *node) { Visit((Node *)node); }
        // virtual void Visit(ExpressionListNode *node) { Visit((Node *)node); }
};

#endif
