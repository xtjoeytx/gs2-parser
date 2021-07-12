#pragma once

#ifndef ASTVISITOR_H
#define ASTVISITOR_H

class Node;
class StatementNode;
class StatementBlock;
class StatementIfNode;
class StatementFnDeclNode;
class StatementContinueNode;
class StatementBreakNode;
class StatementReturnNode;
class StatementNewNode;
class StatementForNode;
class StatementForEachNode;
class StatementSwitchNode;
class StatementWhileNode;
class StatementWithNode;
class ExpressionNode;
class ExpressionFnCallNode;
class ExpressionIdentifierNode;
class ExpressionIntegerNode;
class ExpressionStringConstNode;
class ExpressionBinaryOpNode;
class ExpressionUnaryOpNode;
class ExpressionObjectAccessNode;
class ExpressionListNode;

class NodeVisitor
{
    public:
        virtual void Visit(Node *node) = 0;
        virtual void Visit(StatementNode *node) = 0;
        virtual void Visit(StatementBlock *node) = 0;
        virtual void Visit(StatementIfNode *node) = 0;
        virtual void Visit(ExpressionFnCallNode *node) = 0;
        virtual void Visit(StatementFnDeclNode *node) = 0;
        virtual void Visit(StatementNewNode *node) = 0;
        virtual void Visit(StatementBreakNode *node) = 0;
        virtual void Visit(StatementContinueNode *node) = 0;
        virtual void Visit(StatementReturnNode *node) = 0;
        virtual void Visit(StatementForNode *node) = 0;
        virtual void Visit(StatementForEachNode *node) = 0;
        virtual void Visit(StatementSwitchNode *node) = 0;
        virtual void Visit(StatementWhileNode *node) = 0;
        virtual void Visit(StatementWithNode *node) = 0;
        virtual void Visit(ExpressionNode *node) = 0;
        virtual void Visit(ExpressionIdentifierNode *node) = 0;
        virtual void Visit(ExpressionStringConstNode *node) = 0;
        virtual void Visit(ExpressionIntegerNode *node) = 0;
        virtual void Visit(ExpressionBinaryOpNode *node) = 0;
        virtual void Visit(ExpressionUnaryOpNode *node) = 0;
        virtual void Visit(ExpressionObjectAccessNode *node) = 0;
        virtual void Visit(ExpressionListNode *node) = 0;
};


#endif
