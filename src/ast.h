#pragma once

#ifndef AST_H
#define AST_H

#include <cassert>
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

#define DBGALLOCATIONS

#ifdef DBGALLOCATIONS
void checkForNodeLeaks();
void checkNodeOwnership();
#endif

enum class ExpressionType
{
	EXPR_ANY,
	EXPR_INTEGER,
	EXPR_NUMBER,
	EXPR_STRING,
	EXPR_IDENT,
	EXPR_OBJECT,
	EXPR_ARRAY,
	EXPR_MULTIARRAY,
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

		case ExpressionOp::ConcatAssign:
			return "@=";

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

	Node *parent;

	void takeOwnership(Node* child)
	{
		//assert(child);

		if (child != nullptr)
			child->parent = this;
	}

	template<typename ...Nodes>
	void takeOwnership(Node* child, Nodes&&... node)
	{
		takeOwnership(child);
		(takeOwnership(node), ...);
	}
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

	ExpressionNode() : StatementNode(), isAssignment(false) { }

	virtual ~ExpressionNode() {}

	virtual std::string toString() const = 0;

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_ANY;
	}

	bool isAssignment;
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

	ExpressionPostfixNode(ExpressionNode *firstNode)
		: ExpressionNode()
	{
		assert(firstNode);
		addNode(firstNode);
	}

	virtual ~ExpressionPostfixNode();

	virtual std::string toString() const {
		std::string str;
		for (const auto& n : nodes)
			str.append(n->toString());
		return str;
	}

	virtual ExpressionType expressionType() const {
		return nodes.back()->expressionType();
	}

	void addNode(ExpressionNode* node)
	{
		node->parent = this;
		nodes.push_back(node);
	}

	ExpressionNode * lastNode() const {
		return nodes.back();
	}
	
//protected:
	std::vector<ExpressionNode*> nodes;
};

class ExpressionArrayIndexNode : public ExpressionNode
{
public:
	_NodeName("ExpressionArrayIndexNode")

	ExpressionArrayIndexNode(std::vector<ExpressionNode *> *list)
		: ExpressionNode()
	{
		if (list)
		{
			exprList = std::move(*list);
			delete list;
		}

		for (const auto& expr : exprList)
			takeOwnership(expr);
	}

	virtual ~ExpressionArrayIndexNode();

	virtual std::string toString() const {
		std::string str;
		str.append("[");
		for (const auto& ex : exprList)
		{
			str.append(ex->toString());
			str.append(",");
		}
		str.pop_back();
		str.append("]");
		return str;
	}

	virtual ExpressionType expressionType() const {
		return (exprList.size() > 1 ? ExpressionType::EXPR_MULTIARRAY : ExpressionType::EXPR_ARRAY);
	}

	std::vector<ExpressionNode *> exprList;
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
		takeOwnership(expr);
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
		takeOwnership(expr);
		takeOwnership(lower);
		takeOwnership(higher);
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

class ExpressionTernaryOpNode : public ExpressionNode
{
public:
	_NodeName("ExpressionTernaryOpNode");

	ExpressionTernaryOpNode(ExpressionNode *cond, ExpressionNode *left, ExpressionNode *right)
		: ExpressionNode(), condition(cond), leftExpr(left), rightExpr(right)
	{
		takeOwnership(cond, left, right);
	}

	virtual ~ExpressionTernaryOpNode() { }

	virtual std::string toString() const {
		std::string ret;
		ret.append("(").append(condition->toString());
		ret.append(" ? ").append(leftExpr->toString());
		ret.append(" : ").append(rightExpr->toString());
		ret.append(")");
		return ret;
	}

	ExpressionNode *condition;
	ExpressionNode *leftExpr;
	ExpressionNode *rightExpr;
};

class ExpressionBinaryOpNode : public ExpressionNode
{
	public:
		_NodeName("ExpressionBinaryOpNode");

		ExpressionBinaryOpNode(ExpressionNode *l, ExpressionNode *r, ExpressionOp op, bool assign = false)
			: ExpressionNode(), left(l), right(r), op(op), assignment(assign)
		{
			takeOwnership(left, right);

			if (assign)
			{
				left->isAssignment = true;
				isAssignment = true;
			}
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

			if (right)
				ret += left->toString() + " " + ExpressionOpToString(op) + " " + right->toString();

			if (!assignment)
				ret += ")";

			return ret;
		}

		virtual ExpressionType expressionType() const {
			return left->expressionType();
		}
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
			takeOwnership(e);
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

	ExpressionFnCallNode(ExpressionNode *funcExpr, ExpressionNode *objExpr, std::vector<ExpressionNode *> *argList = nullptr)
		: ExpressionNode(), funcExpr(funcExpr), objExpr(objExpr), discardReturnValue(false)
	{
		if (argList)
		{
			args = std::move(*argList);
			delete argList;
		}

		takeOwnership(funcExpr, objExpr);
		for (const auto& node : args)
			takeOwnership(node);
	}

	virtual ~ExpressionFnCallNode();

	virtual std::string toString() const
	{
		std::string argList;
		for (const auto& arg : args)
		{
			argList += arg->toString();
			argList += ",";
		}

		if (!argList.empty())
			argList.pop_back();
		
		return std::string(funcExpr->toString()) + "(" + argList + ")";
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_FUNCTION;
	}

	ExpressionNode* funcExpr;
	ExpressionNode* objExpr;
	std::vector<ExpressionNode*> args;
	bool discardReturnValue;
};

class ExpressionNewArrayNode : public ExpressionNode
{
public:
	_NodeName("ExpressionNewArrayNode");

	ExpressionNewArrayNode(std::vector<int> *dim = nullptr)
		: ExpressionNode()
	{
		if (dim)
		{
			dimensions = std::move(*dim);
			delete dim;
		}
	}

	virtual ~ExpressionNewArrayNode() {}

	virtual std::string toString() const {
		std::string str("new");
		for (const auto& dim : dimensions)
			str.append("[").append(std::to_string(dim)).append("]");

		return str;
	}

	std::vector<int> dimensions;
};

class ExpressionNewObjectNode : public ExpressionNode
{
public:
	_NodeName("ExpressionNewNode");

	ExpressionNewObjectNode(ExpressionNode *newExpr, std::vector<ExpressionNode*> *argList = 0)
		: ExpressionNode(), newExpr(newExpr)
	{
		if (argList)
		{
			args = std::move(*argList);
			delete argList;
		}

		takeOwnership(newExpr);
		for (const auto& node : args)
			takeOwnership(node);
	}

	virtual ~ExpressionNewObjectNode();

	virtual std::string toString() const {
		std::string str = "new ";
		return str + newExpr->toString();
	}

	ExpressionNode *newExpr;
	std::vector<ExpressionNode *> args;
};

class ExpressionListNode : public ExpressionNode
{
public:
	_NodeName("ExpressionListNode")

	ExpressionListNode(std::vector<ExpressionNode *> *argList)
		: ExpressionNode()
	{
		if (argList)
		{
			args = std::move(*argList);
			delete argList;
		}

		for (const auto& node : args)
			takeOwnership(node);
	}

	virtual ~ExpressionListNode();

	virtual std::string toString() const
	{
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
		takeOwnership(expr, thenBlock, elseBlock);
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
		: StatementNode(), stmtBlock(block), pub(false), ident(std::string(id)), objectName(std::move(objName))
	{
		if (argList)
		{
			args = std::move(*argList);
			delete argList;
		}

		takeOwnership(stmtBlock);
		for (const auto& node : args)
			takeOwnership(node);
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

	StatementNewNode(const char *objName, std::vector<ExpressionNode *> *argList, StatementBlock *block)
		: StatementNode(), stmtBlock(block), ident(std::string(objName))
	{
		if (argList)
		{
			args = std::move(*argList);
			delete argList;
		}

		takeOwnership(stmtBlock);
		for (const auto& node : args)
			takeOwnership(node);
	}
	virtual ~StatementNewNode();

	std::string ident;
	StatementBlock *stmtBlock;
	std::vector<ExpressionNode *> args;
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
		takeOwnership(expr);
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
		takeOwnership(expr, block);
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
		takeOwnership(expr, block);
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
		takeOwnership(init, cond, postop, block);
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
		takeOwnership(name, expr, block);
	}

	virtual ~StatementForEachNode();

	ExpressionNode *name;
	ExpressionNode *expr;
	StatementNode *block;
};

class SwitchCaseState
{
public:
	SwitchCaseState(StatementBlock* stmtBlock) 
		: block(stmtBlock) { }

	SwitchCaseState(SwitchCaseState&& o) noexcept
	{
		block = o.block;
		exprList = std::move(o.exprList);
	}

	SwitchCaseState(const SwitchCaseState&) = delete;
	SwitchCaseState& operator=(const SwitchCaseState&) = delete;

	StatementBlock *block;
	std::vector<ExpressionNode *> exprList;
};

class StatementSwitchNode : public StatementNode
{
public:
	_NodeName("StatementSwitchNode")

	StatementSwitchNode(ExpressionNode *expr, std::vector<SwitchCaseState> *caseNodes)
		: StatementNode(), expr(expr)
	{
		if (caseNodes)
		{
			cases = std::move(*caseNodes);
			delete caseNodes;
		}

		takeOwnership(expr);
		for (const auto& caseNode : cases)
		{
			takeOwnership(caseNode.block);
			for (const auto& matchExpr : caseNode.exprList)
				takeOwnership(matchExpr);
		}
	}

	virtual ~StatementSwitchNode();

	ExpressionNode *expr;
	std::vector<SwitchCaseState> cases;
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
