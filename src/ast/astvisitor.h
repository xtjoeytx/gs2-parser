#pragma once

#ifndef ASTVISITOR_H
#define ASTVISITOR_H

class NodeVisitor
{
    public:
        virtual void Visit(class Node *node) = 0;
        virtual void Visit(class StatementNode *node) = 0;
        virtual void Visit(class StatementBlock *node) = 0;
        virtual void Visit(class StatementIfNode *node) = 0;
        virtual void Visit(class StatementFnDeclNode *node) = 0;
        virtual void Visit(class StatementNewNode *node) = 0;
        virtual void Visit(class StatementBreakNode *node) = 0;
        virtual void Visit(class StatementContinueNode *node) = 0;
        virtual void Visit(class StatementReturnNode *node) = 0;
        virtual void Visit(class StatementForNode *node) = 0;
        virtual void Visit(class StatementForEachNode *node) = 0;
        virtual void Visit(class StatementSwitchNode *node) = 0;
        virtual void Visit(class StatementWhileNode *node) = 0;
        virtual void Visit(class StatementWithNode *node) = 0;
        virtual void Visit(class ExpressionNode *node) = 0;
        virtual void Visit(class ExpressionIdentifierNode *node) = 0;
        virtual void Visit(class ExpressionStringConstNode *node) = 0;
        virtual void Visit(class ExpressionIntegerNode *node) = 0;
        virtual void Visit(class ExpressionNumberNode *node) = 0;
        virtual void Visit(class ExpressionPostfixNode *node) = 0;
        virtual void Visit(class ExpressionInOpNode *node) = 0;
        virtual void Visit(class ExpressionCastNode *node) = 0;
        virtual void Visit(class ExpressionArrayIndexNode* node) = 0;
        virtual void Visit(class ExpressionFnCallNode *node) = 0;
        virtual void Visit(class ExpressionNewArrayNode *node) = 0;
        virtual void Visit(class ExpressionNewObjectNode *node) = 0;
        virtual void Visit(class ExpressionTernaryOpNode *node) = 0;
        virtual void Visit(class ExpressionBinaryOpNode *node) = 0;
        virtual void Visit(class ExpressionUnaryOpNode *node) = 0;
        virtual void Visit(class ExpressionStrConcatNode *node) = 0;
        virtual void Visit(class ExpressionListNode *node) = 0;
        virtual void Visit(class ExpressionConstantNode *node) = 0;
};

#endif
