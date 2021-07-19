#pragma once

#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

#include "astvisitor.h"

#define _NodeName(name) \
	inline static const char * NodeName = name; \
	virtual const char * NodeType() const { \
		return NodeName; \
	} \
	virtual void visit(NodeVisitor *v) { v->Visit(this); }


enum class ExpressionType
{
	EXPR_ANY,
	EXPR_INTEGER,
	EXPR_NUMBER,
	EXPR_STRING,
	EXPR_OBJECT
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

#ifdef DBGALLOCATIONS
static int count = 0;
#endif

class Node
{
public:
	Node() {
#ifdef DBGALLOCATIONS
		printf("Count: %d\n", ++count);
#endif
	}
	virtual ~Node() { 
#ifdef DBGALLOCATIONS
		printf("Count: %d\n", --count);
#endif
	}

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

	virtual std::string toString() const = 0;

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_ANY;
	}

	std::vector<ExpressionNode *> nodes;
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

class ExpressionArrayIndexNode : public ExpressionNode
{
public:
	_NodeName("ExpressionArrayIndexNode")

	ExpressionArrayIndexNode(ExpressionNode *expr, ExpressionNode *idx)
	: ExpressionNode(), expr(expr), idx(idx)
	{
	}
	virtual ~ExpressionArrayIndexNode() {}

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
};

class ExpressionUnaryOpNode : public ExpressionNode
{
	public:
		_NodeName("ExpressionUnaryOpNode");

		ExpressionUnaryOpNode(ExpressionNode *e, ExpressionOp op, bool opFirst)
			: ExpressionNode(), expr(e), op(op), opFirst(opFirst)
		{

		}

		virtual ~ExpressionUnaryOpNode();

		ExpressionNode *expr;
		ExpressionOp op;
		bool opFirst;

		virtual std::string toString() const
		{
			if (opFirst)
				return std::string(ExpressionOpToString(op)) + expr->toString();
			
			return expr->toString() + std::string(ExpressionOpToString(op));
		}
};

class ExpressionObjectAccessNode : public ExpressionNode
{
	public:
		_NodeName("ExpressionObjectAccessNode")

		ExpressionObjectAccessNode(ExpressionNode *l, ExpressionNode *r)
			: ExpressionNode(), left(l), right(r)
		{
			//nodes.push_back(reinterpret_cast<ExpressionIdentifierNode *>(l));
			//nodes.push_back(reinterpret_cast<ExpressionIdentifierNode *>(r));
		}

		virtual ~ExpressionObjectAccessNode();

		virtual std::string toString() const {
			std::string str(left->toString());
			for (const auto& n : nodes)
				str.append(".").append(n->toString());

			if (right)
				str.append(".").append(right->toString());

			return str;
		}

		ExpressionNode* left;
		ExpressionNode* right;
		std::vector<ExpressionNode*> nodes;
};

class ExpressionFnCallNode : public ExpressionNode
{
public:
	_NodeName("ExpressionFnCallNode")

	ExpressionFnCallNode(ExpressionNode *funcExpr, ExpressionObjectAccessNode *objExpr, std::vector<ExpressionNode *> *args = 0)
		: ExpressionNode(), funcExpr(funcExpr), objExpr(objExpr), args(args), discardReturnValue(false)
	{
		if (objExpr && funcExpr == objExpr->right) {
			objExpr->right = nullptr;
			printf("unset\n");
		}
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

	ExpressionNode* funcExpr;
	ExpressionObjectAccessNode * objExpr;
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

	ExpressionListNode(std::vector<ExpressionNode *> *a) : args(a) {

	}

	virtual ~ExpressionListNode();

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

	StatementFnDeclNode(const char *id, std::vector<ExpressionNode *> *a, StatementBlock *block)
		: StatementNode(), stmtBlock(block), args(a), pub(false)
	{
		ident = std::string(id);
	}
	
	virtual ~StatementFnDeclNode();

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
		: StatementNode(), expr(expr), cases(caseNodes)
	{

	}

	virtual ~StatementSwitchNode();

	ExpressionNode *expr;
	std::vector<CaseNode *> *cases;
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
