#pragma once

#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

#include "astvisitor.h"

#define _NodeName(name) \
    virtual const char * NodeType() const { \
        return name; \
    } \
    virtual void visit(NodeVisitor *v) { v->Visit(this); }


class Node
{
public:
    Node() {}
    virtual ~Node() { }

    virtual const char * NodeType() const = 0;
    virtual void visit(NodeVisitor *v) { v->Visit(this); }
};

class ProgramNode : public Node
{
public:
    _NodeName("ProgramNode")

    ProgramNode() : Node() { }

    std::vector<StatementNode *> nodes;
};

class StatementNode : public Node
{
public:
    _NodeName("StatementNode")

    StatementNode() : Node() { }
};


class ExpressionNode : public StatementNode
{
public:
    _NodeName("ExpressionNode")

    ExpressionNode() : StatementNode() { }

    virtual std::string toString() const = 0;
};

class ExpressionIntegerNode : public ExpressionNode
{
public:
    _NodeName("ExpressionIntegerNode")

    ExpressionIntegerNode(int num) : ExpressionNode()
    {
        val = num;
    }
   virtual std::string toString() const {
       return std::to_string(val);
   }

    int val;
};

class ExpressionIdentifierNode : public ExpressionNode
{
public:
    _NodeName("ExpressionIdentifierNode")

    ExpressionIdentifierNode(const char *str) : ExpressionNode()
    {
        val = std::string(str);
    }
    virtual std::string toString() const {
       return val;
   }

    std::string val;
};

class ExpressionStringConstNode : public ExpressionNode
{
public:
    _NodeName("ExpressionStringConstNode")

    ExpressionStringConstNode(const char *str) : ExpressionNode()
    {
        val = std::string(str);
    }
    virtual std::string toString() const {
       return val;
   }

    std::string val;
};

class ExpressionBinaryOpNode : public ExpressionNode
{
    public:
        _NodeName("ExpressionBinaryOpNode");

        ExpressionBinaryOpNode(ExpressionNode *l, ExpressionNode *r, std::string op, bool assign = false)
            : ExpressionNode(), left(l), right(r), op(op), assignment(assign)
        {

        }

        ExpressionNode *left;
        ExpressionNode *right;
        std::string op;
        bool assignment;

        virtual std::string toString() const {
            std::string ret;

            if (!assignment)
                ret += "(";
            
            ret += left->toString() + " " + op.c_str() + " " + right->toString();

            if (!assignment)
                ret += ")";

            return ret;
        }
};

class ExpressionUnaryOpNode : public ExpressionNode
{
    public:
        _NodeName("ExpressionUnaryOpNode");

        ExpressionUnaryOpNode(ExpressionNode *e, std::string op)
            : ExpressionNode(), expr(e), op(op)
        {

        }

        ExpressionNode *expr;
        std::string op;

        virtual std::string toString() const {
            return std::string(op) + expr->toString();
        }
};

class ExpressionObjectAccessNode : public ExpressionNode
{
    public:
        _NodeName("ExpressionObjectAccessNode")

        ExpressionObjectAccessNode(ExpressionNode *l, ExpressionNode *r)
            : ExpressionNode(), left(l), right(r)
        {

        }

        ExpressionNode *left;
        ExpressionNode *right;

        virtual std::string toString() const {
            return std::string(left->toString()) + "." + std::string(right->toString());
        }
};


class ExpressionFnCallNode : public ExpressionNode
{
public:
    _NodeName("ExpressionFnCallNode")

    ExpressionFnCallNode(ExpressionNode *e, std::vector<ExpressionNode *> *a = 0)
        : ExpressionNode(), expr(e), args(a)
    {
    }
    virtual ~ExpressionFnCallNode() { }

    ExpressionNode *expr;
    std::vector<ExpressionNode *> *args;

    virtual std::string toString() const {
        std::string argList;
        if (args)
        {
            for (const auto& s : *args)
            {
                argList += s->toString();
                argList += ",";
            }
            argList.pop_back();
        }

        return std::string(expr->toString()) + "(" + argList + ")";
    }
};

class ExpressionListNode : public ExpressionNode
{
public:
    _NodeName("ExpressionListNode")

    ExpressionListNode(std::vector<ExpressionNode *> *a) : args(a) {

    }

    virtual std::string toString() const {
        std::string argList;
        if (args)
        {
            for (const auto& s : *args)
            {
                argList += s->toString();
                argList += ",";
            }
            argList.pop_back();
        }

        return std::string("{") + argList + "}";

    }

    std::vector<ExpressionNode *> *args;
};

class StatementBlock : public StatementNode
{
public:
    _NodeName("StatementBlock")

    StatementBlock(StatementNode *node = 0) : StatementNode() {
        append(node);
    }

    virtual ~StatementBlock() { }

    void append(StatementNode *node) {
        if (node) {
            statements.push_back(node);
        }
    }

    std::vector<StatementNode *> statements;
};

class StatementIfNode : public StatementNode
{
public:
    _NodeName("StatementIfNode")

    StatementIfNode(ExpressionNode *expr, StatementNode *thenBlock, StatementNode *elseBlock = nullptr)
        : StatementNode(), expr(expr), thenBlock(thenBlock), elseBlock(elseBlock)
    {

    }
    virtual ~StatementIfNode() { }

    ExpressionNode *expr;
    StatementNode *thenBlock;
    StatementNode *elseBlock;
};

class StatementFnDeclNode : public StatementNode
{
public:
    _NodeName("StatementFnDeclNode")

    StatementFnDeclNode(const char *id, std::vector<ExpressionNode *> *a, StatementBlock *block)
        : StatementNode(), stmtBlock(block), args(a), pub(false)
    {
        ident = std::string(id);
    }
    virtual ~StatementFnDeclNode() { }

    void setPublic(bool r) {
        pub = r;
    }

    bool pub;
    std::string ident;
    StatementBlock *stmtBlock;
    std::vector<ExpressionNode *> *args;
};

class StatementNewNode : public StatementNode
{
public:
    _NodeName("StatementNewNode")

    StatementNewNode(const char *objname, std::vector<ExpressionNode *> *a, StatementBlock *block)
        : StatementNode(), stmtBlock(block), args(a)
    {
        ident = std::string(objname);
    }
    virtual ~StatementNewNode() { }

    std::string ident;
    StatementBlock *stmtBlock;
    std::vector<ExpressionNode *> *args;
};

class StatementBreakNode : public StatementNode
{
public:
    _NodeName("StatementBreakNode")

    StatementBreakNode()
        : StatementNode()
    {

    }
};

class StatementContinueNode : public StatementNode
{
public:
    _NodeName("StatementContinueNode")

    StatementContinueNode()
        : StatementNode()
    {

    }
};

class StatementReturnNode : public StatementNode
{
public:
    _NodeName("StatementReturnNode")

    StatementReturnNode(ExpressionNode *expr)
        : StatementNode(), expr(expr)
    {

    }
    
    ExpressionNode *expr;
};

class StatementWhileNode : public StatementNode
{
public:
    _NodeName("StatementWhileNode")

    StatementWhileNode(ExpressionNode *expr, StatementNode *block)
        : StatementNode(), expr(expr), block(block)
    {

    }
    virtual ~StatementWhileNode() { }

    ExpressionNode *expr;
    StatementNode *block;
};

class StatementWithNode : public StatementNode
{
public:
    _NodeName("StatementWithNode")

    StatementWithNode(ExpressionNode *expr, StatementNode *block)
        : StatementNode(), expr(expr), block(block)
    {

    }

    ExpressionNode *expr;
    StatementNode *block;
};

class StatementForNode : public StatementNode
{
public:
    _NodeName("StatementForNode")

    StatementForNode(ExpressionNode *init, ExpressionNode *cond, ExpressionNode *incr, StatementNode *block)
        : StatementNode(), init(init), cond(cond), postop(incr), block(block)
    {

    }
    
    ExpressionNode *init;
    ExpressionNode *cond;
    ExpressionNode *postop;
    StatementNode *block;
};

class StatementForEachNode : public StatementNode
{
public:
    _NodeName("StatementForEachNode")

    StatementForEachNode(ExpressionNode *name, ExpressionNode *expr, StatementNode *block)
        : StatementNode(), name(name), expr(expr), block(block)
    {

    }
    
    ExpressionNode *name;
    ExpressionNode *expr;
    StatementNode *block;
};

class CaseNode
{
public:
    CaseNode(ExpressionNode *expr, StatementNode *stmt)
        : expr(expr), stmt(stmt)
    {

    }

    ExpressionNode *expr;
    StatementNode *stmt;
};

class StatementSwitchNode : public StatementNode
{
public:
    _NodeName("StatementSwitchNode")

    StatementSwitchNode(ExpressionNode *expr, std::vector<CaseNode *> *caseNodes)
        : StatementNode(), expr(expr), cases(caseNodes)
    {

    }

    ExpressionNode *expr;
    std::vector<CaseNode *> *cases;
};

#endif
