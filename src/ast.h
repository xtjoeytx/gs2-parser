#pragma once

#ifndef AST_H
#define AST_H

#include <cstring>
#include <string>
#include <vector>

#include "astvisitor.h"

#define _NodeName(name) \
	inline static const char * NodeName = name; \
	virtual const char * NodeType() const { \
		return NodeName; \
	} \
	virtual void visit(NodeVisitor *v) { v->Visit(this); }

//#define DBGALLOCATIONS

#ifdef DBGALLOCATIONS
void checkForNodeLeaks();
#endif

enum class ExpressionType
{
	EXPR_ANY,
	EXPR_INTEGER,
	EXPR_NUMBER,
	EXPR_STRING,
	EXPR_IDENT,
	EXPR_OBJECT,
	EXPR_FUNCTION
};

enum class ExpressionOp {
	Plus,
	Minus,
	Multiply,
	Divide,
	Mod,
	Pow,
	Assign,
	Concat,
	Equal,
	NotEqual,
	LessThan,
	GreaterThan,
	LessThanOrEqual,
	GreaterThanOrEqual,
	LogicalAnd,
	LogicalOr,

	PlusAssign,
	MinusAssign,
	MultiplyAssign,
	DivideAssign,
	PowAssign,
	ModAssign,
	ConcatAssign,

	UnaryStringCast,
	UnaryNot,
	UnaryMinus,
	Increment,
	Decrement
};

inline const char* ExpressionOpToString(ExpressionOp op)
{
	switch (op)
	{
		case ExpressionOp::Plus:
			return "+";

		case ExpressionOp::Minus:
			return "-";

		case ExpressionOp::Multiply:
			return "*";

		case ExpressionOp::Divide:
			return "/";

		case ExpressionOp::Mod:
			return "%";

		case ExpressionOp::Pow:
			return "^";

		case ExpressionOp::Assign:
			return "=";

		case ExpressionOp::Concat:
		case ExpressionOp::UnaryStringCast:
			return "@";

		case ExpressionOp::Equal:
			return "==";

		case ExpressionOp::NotEqual:
			return "!=";

		case ExpressionOp::LessThan:
			return "<";

		case ExpressionOp::GreaterThan:
			return ">";

		case ExpressionOp::LessThanOrEqual:
			return "<=";

		case ExpressionOp::GreaterThanOrEqual:
			return ">=";

		case ExpressionOp::LogicalAnd:
			return "&&";

		case ExpressionOp::LogicalOr:
			return "||";

		case ExpressionOp::UnaryNot:
			return "!";

		case ExpressionOp::UnaryMinus:
			return "-";

		case ExpressionOp::Increment:
			return "++";

		case ExpressionOp::Decrement:
			return "--";

		default:
			return "Unknown";
	}
}

class Node
{
public:
	Node();

	virtual ~Node();

	virtual const char * NodeType() const = 0;
	virtual void visit(NodeVisitor *v) { v->Visit(this); }
};

class ProgramNode : public Node
{
public:
	_NodeName("ProgramNode")

	ProgramNode() : Node() { }
	virtual ~ProgramNode();

	std::vector<StatementNode *> nodes;
};

class StatementNode : public Node
{
public:
	_NodeName("StatementNode")

	StatementNode() : Node() { }
	virtual ~StatementNode() = default;
};

class ExpressionNode : public StatementNode
{
public:
	_NodeName("ExpressionNode")

	ExpressionNode() : StatementNode() { }

	virtual ~ExpressionNode() {}

	virtual std::string toString() const = 0;

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_ANY;
	}
};

class ExpressionIntegerNode : public ExpressionNode
{
public:
	_NodeName("ExpressionIntegerNode")

	ExpressionIntegerNode(int num)
		: ExpressionNode()
	{
		val = num;
	}

	virtual ~ExpressionIntegerNode() { }

	virtual std::string toString() const {
		return std::to_string(val);
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_INTEGER;
	}

	int val;
};

class ExpressionNumberNode : public ExpressionNode
{
public:
	_NodeName("ExpressionNumberNode")

	ExpressionNumberNode(const char *str)
		: ExpressionNode()
	{
		val = std::string(str);
	}

	virtual ~ExpressionNumberNode() { }

	virtual std::string toString() const {
		return val;
	}

	std::string val;
};

class ExpressionIdentifierNode : public ExpressionNode
{
public:
	_NodeName("ExpressionIdentifierNode")

	ExpressionIdentifierNode(const char *str)
		: ExpressionNode()
	{
		val = std::string(str);
	}

	virtual std::string toString() const {
		return val;
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_IDENT;
	}

	std::string val;
};

class ExpressionStringConstNode : public ExpressionNode
{
public:
	_NodeName("ExpressionStringConstNode")

	ExpressionStringConstNode(const char *str)
		: ExpressionNode()
	{
		val = std::string(str);
	}

	virtual std::string toString() const {
		return val;
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_STRING;
	}

	std::string val;
};

class ExpressionPostfixNode : public ExpressionNode
{
public:
	_NodeName("ExpressionPostfixNode")

	ExpressionPostfixNode(ExpressionNode *parent)
		: ExpressionNode()
	{
		nodes.push_back(parent);
	}

	virtual ~ExpressionPostfixNode();

	virtual std::string toString() const {
		return "-";
	}

	virtual ExpressionType expressionType() const {
		return nodes.back()->expressionType();
	}

	std::vector<ExpressionNode*> nodes;
};

class ExpressionArrayIndexNode : public ExpressionNode
{
public:
	_NodeName("ExpressionArrayIndexNode")

	ExpressionArrayIndexNode(ExpressionNode *expr, ExpressionNode *idx)
	: ExpressionNode(), expr(expr), idx(idx)
	{
	}

	virtual ~ExpressionArrayIndexNode();

	virtual std::string toString() const {
		return expr->toString().append("[").append(idx->toString()).append("]");;
	}

	ExpressionNode* expr;
	ExpressionNode* idx;
};

class ExpressionCastNode : public ExpressionNode
{
public:
	enum class CastType
	{
		INTEGER,
		FLOAT,
		STRING
	};

public:
	_NodeName("ExpressionCastNode")

	ExpressionCastNode(ExpressionNode* expr, CastType type)
		: ExpressionNode(), expr(expr), type(type)
	{
	}
	virtual ~ExpressionCastNode();

	virtual std::string toString() const {
		return expr->toString();
	}

	ExpressionNode* expr;
	CastType type;
};

class ExpressionInOpNode : public ExpressionNode
{
public:
	_NodeName("ExpressionInOpNode");

	ExpressionInOpNode(ExpressionNode* expr, ExpressionNode* lower, ExpressionNode* higher)
		: ExpressionNode(), expr(expr), lower(lower), higher(higher)
	{

	}

	virtual ~ExpressionInOpNode();

	virtual std::string toString() const {
		return "in op";
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_INTEGER;
	}

	ExpressionNode* expr;
	ExpressionNode* lower;
	ExpressionNode* higher;
};

class ExpressionBinaryOpNode : public ExpressionNode
{
	public:
		_NodeName("ExpressionBinaryOpNode");

		ExpressionBinaryOpNode(ExpressionNode *l, ExpressionNode *r, ExpressionOp op, bool assign = false)
			: ExpressionNode(), left(l), right(r), op(op), assignment(assign)
		{

		}

		virtual ~ExpressionBinaryOpNode();

		ExpressionNode *left;
		ExpressionNode *right;
		ExpressionOp op;
		bool assignment;

		virtual std::string toString() const {
			std::string ret;

			if (!assignment)
				ret += "(";

			ret += left->toString() + " " + ExpressionOpToString(op) + " " + right->toString();

			if (!assignment)
				ret += ")";

			return ret;
		}

		//virtual ExpressionType expressionType() const {
		//	return ExpressionType::EXPR_INTEGER;
		//}
};

class ExpressionStrConcatNode : public ExpressionBinaryOpNode
{
public:
	_NodeName("ExpressionStrConcatNode");

	ExpressionStrConcatNode(ExpressionNode* l, ExpressionNode* r, char sep = 0)
		: ExpressionBinaryOpNode(l, r, ExpressionOp::Concat), sep(sep)
	{

	}

	virtual ~ExpressionStrConcatNode() {}

	char sep;
};

class ExpressionUnaryOpNode : public ExpressionNode
{
	public:
		_NodeName("ExpressionUnaryOpNode");

		ExpressionUnaryOpNode(ExpressionNode *e, ExpressionOp op, bool opFirst)
			: ExpressionNode(), expr(e), op(op), opFirst(opFirst), opUnused(false)
		{

		}

		virtual ~ExpressionUnaryOpNode();

		ExpressionNode *expr;
		ExpressionOp op;
		bool opFirst;
		bool opUnused;

		virtual std::string toString() const
		{
			if (opFirst)
				return std::string(ExpressionOpToString(op)) + expr->toString();

			return expr->toString() + std::string(ExpressionOpToString(op));
		}
};

class ExpressionFnCallNode : public ExpressionNode
{
public:
	_NodeName("ExpressionFnCallNode")

	ExpressionFnCallNode(ExpressionNode *funcExpr, ExpressionNode*objExpr, std::vector<ExpressionNode *> *args = 0)
		: ExpressionNode(), funcExpr(funcExpr), objExpr(objExpr), args(args), discardReturnValue(false)
	{
	}

	virtual ~ExpressionFnCallNode();

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

		return std::string(funcExpr->toString()) + "(" + argList + ")";
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_FUNCTION;
	}

	ExpressionNode* funcExpr;
	ExpressionNode* objExpr;
	std::vector<ExpressionNode*>* args;
	bool discardReturnValue;
};

class ExpressionNewNode : public ExpressionNode
{
public:
	_NodeName("ExpressionNewNode");

	ExpressionNewNode(ExpressionNode *newExpr, std::vector<ExpressionNode*>* args = 0)
		: ExpressionNode(), newExpr(newExpr), args(args)
	{

	}

	virtual ~ExpressionNewNode();

	virtual std::string toString() const {
		std::string str = "new ";
		return str + newExpr->toString();
	}

	ExpressionNode* newExpr;
	std::vector<ExpressionNode*>* args;
};

class ExpressionListNode : public ExpressionNode
{
public:
	_NodeName("ExpressionListNode")

	ExpressionListNode(std::vector<ExpressionNode *> *argList)
	{
		if (argList)
		{
			args = std::move(*argList);
			delete argList;
		}
	}

	virtual ~ExpressionListNode();

	virtual std::string toString() const {
		std::string argList;
		if (!args.empty())
		{
			for (const auto& s : args)
			{
				argList += s->toString();
				argList += ",";
			}
			argList.pop_back();
		}

		return std::string("{") + argList + "}";

	}

	std::vector<ExpressionNode *> args;
};

class StatementBlock : public StatementNode
{
public:
	_NodeName("StatementBlock")

	StatementBlock(StatementNode *node = 0)
		: StatementNode()
	{
		append(node);
	}

	virtual ~StatementBlock();

	void append(StatementNode *node);

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

	virtual ~StatementIfNode();

	ExpressionNode *expr;
	StatementNode *thenBlock;
	StatementNode *elseBlock;
};

class StatementFnDeclNode : public StatementNode
{
public:
	_NodeName("StatementFnDeclNode")

	StatementFnDeclNode(const char *id, std::vector<ExpressionNode *> *argList, StatementBlock *block, std::string objName = "")
		: StatementNode(), stmtBlock(block), pub(false), objectName(std::move(objName))
	{
		if (argList)
		{
			args = std::move(*argList);
			delete argList;
		}

		ident = std::string(id);
	}

	virtual ~StatementFnDeclNode();

	void setPublic(bool r) {
		pub = r;
	}

	bool pub;
	std::string ident, objectName;
	StatementBlock *stmtBlock;
	std::vector<ExpressionNode *> args;
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
	virtual ~StatementNewNode();

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

	virtual ~StatementReturnNode();

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
	virtual ~StatementWhileNode();

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

	virtual ~StatementWithNode();

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

	virtual ~StatementForNode();

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

	virtual ~StatementForEachNode();

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

	virtual ~CaseNode();

	ExpressionNode *expr;
	StatementNode *stmt;
};

class StatementSwitchNode : public StatementNode
{
public:
	_NodeName("StatementSwitchNode")

	StatementSwitchNode(ExpressionNode *expr, std::vector<CaseNode *> *caseNodes)
		: StatementNode(), expr(expr)
	{
		if (caseNodes)
		{
			cases = std::move(*caseNodes);
			delete caseNodes;
		}
	}

	virtual ~StatementSwitchNode();

	ExpressionNode *expr;
	std::vector<CaseNode *> cases;
};

struct EnumMember
{
	std::string node;
	bool hasIndex;
	int idx;

	EnumMember(std::string node)
		: node(std::move(node)), idx(0), hasIndex(false)
	{

	}

	EnumMember(std::string node, int idx)
		: node(std::move(node)), idx(idx), hasIndex(true)
	{

	}
};

class EnumList
{
	public:
		EnumList(EnumMember *member)
			: curIdx(0)
		{
			addMember(member);
		}

		~EnumList()
		{
			for (const auto& n : members)
				delete n;
		}

		void addMember(EnumMember *member);

		const std::vector<EnumMember *>& getMembers() const {
			return members;
		}

	private:
		int curIdx;
		std::vector<EnumMember *> members;
};

#endif
