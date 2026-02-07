#pragma once

#ifndef AST_H
#define AST_H

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include "ast/expressiontypes.h"
#include "ast/NodeVisitor.h"

#define _NodeName(name) \
	inline static const char * NodeName = name; \
	virtual const char * NodeType() const { \
		return NodeName; \
	} \
	virtual void visit(NodeVisitor *v) { v->Visit(this); }


//#define DBGALLOCATIONS
#ifdef DBGALLOCATIONS
void checkForNodeLeaks();
void checkNodeOwnership();
#endif

class Node
{
public:
	Node();
	virtual ~Node();

	virtual const char * NodeType() const = 0;
	virtual void visit(NodeVisitor *v) { v->Visit(this); }
	virtual bool isExpressionNode() const { return false; }
	virtual bool isStatementNode() const { return false; }

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

	Node *parent;
};

class StatementNode : public Node
{
public:
	_NodeName("StatementNode")

	StatementNode() : Node() { }

	virtual bool isStatementNode() const { return true; }
};

class ExpressionNode : public StatementNode
{
public:
	_NodeName("ExpressionNode")

	ExpressionNode() : StatementNode(), isAssignment(false) { }

	virtual bool isExpressionNode() const {
		return true;
	}

	virtual std::string toString() const = 0;
	virtual ExpressionType expressionType() const = 0;

	bool isAssignment;
};

class ExpressionConstantNode : public ExpressionNode
{
public:
	_NodeName("ExpressionConstantNode")

	enum class ConstantType
	{
		TRUE_T,
		FALSE_T,
		NULL_T,
	};

	ExpressionConstantNode(ConstantType val)
		: ExpressionNode(), type(val)
	{
	}

	virtual std::string toString() const {
		return std::to_string(int(type));
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_INTEGER;
	}

	ConstantType type;
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

	ExpressionNumberNode(std::string *str)
		: ExpressionNode(), val(str)
	{
	}

	virtual std::string toString() const {
		return *val;
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_NUMBER;
	}

	std::string *val;
};

class ExpressionIdentifierNode : public ExpressionNode
{
public:
	_NodeName("ExpressionIdentifierNode")

	ExpressionIdentifierNode(std::string *str)
		: ExpressionNode(), val(str), checkForReservedIdents(true)
	{
	}

	virtual std::string toString() const {
		return *val;
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_IDENT;
	}

	std::string *val;
	bool checkForReservedIdents;
};

class ExpressionStringConstNode : public ExpressionNode
{
public:
	_NodeName("ExpressionStringConstNode")

	ExpressionStringConstNode(std::string *str)
		: ExpressionNode(), val(str)
	{
	}

	virtual std::string toString() const {
		return *val;
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_STRING;
	}
	
	std::string *val;
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

	virtual std::string toString() const {
		std::string str;
		for (const auto& n : nodes)
			str.append(n->toString()).append(".");
		if (!str.empty())
			str.pop_back();
		return str;
	}

	virtual ExpressionType expressionType() const {
		return nodes.back()->expressionType();
	}

	void addNode(ExpressionNode* node)
	{
		assert(node);

		// Only the first identifier can be used for reserved keywords
		if (!nodes.empty() && node->expressionType() == ExpressionType::EXPR_IDENT)
		{
			ExpressionIdentifierNode *identNode = reinterpret_cast<ExpressionIdentifierNode *>(node);
			identNode->checkForReservedIdents = false;
		}

		takeOwnership(node);
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
		STRING,
		TRANSLATION
	};

public:
	_NodeName("ExpressionCastNode")

	ExpressionCastNode(ExpressionNode* expr, CastType type)
		: ExpressionNode(), expr(expr), type(type)
	{
		takeOwnership(expr);
	}
	
	virtual std::string toString() const {
		return expr->toString();
	}

	virtual ExpressionType expressionType() const
	{
		switch (type)
		{
			case CastType::INTEGER:
				return ExpressionType::EXPR_INTEGER;

			case CastType::FLOAT:
				return ExpressionType::EXPR_NUMBER;

			case CastType::STRING:
			case CastType::TRANSLATION:
				return ExpressionType::EXPR_STRING;
		}

		return expr->expressionType();
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

	virtual std::string toString() const
	{
		std::string ret;
		ret.append("(").append(condition->toString());
		ret.append(" ? ").append(leftExpr->toString());
		ret.append(" : ").append(rightExpr->toString());
		ret.append(")");
		return ret;
	}

	virtual ExpressionType expressionType() const
	{
		auto exprType = leftExpr->expressionType();
		if (exprType == rightExpr->expressionType())
			return exprType;

		return ExpressionType::EXPR_ANY;
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

		ExpressionNode *left;
		ExpressionNode *right;
		ExpressionOp op;
		bool assignment;

		virtual std::string toString() const
		{
			std::string ret;

			if (!assignment)
				ret += "(";

			if (right)
				ret += left->toString() + " " + ExpressionOpToString(op) + " " + right->toString();

			if (!assignment)
				ret += ")";

			return ret;
		}

		virtual ExpressionType expressionType() const
		{
			switch (op)
			{
				case ExpressionOp::Plus:
				case ExpressionOp::Minus:
				case ExpressionOp::Multiply:
				case ExpressionOp::Divide:
				case ExpressionOp::Mod:
				case ExpressionOp::Pow:
				case ExpressionOp::Equal:
				case ExpressionOp::NotEqual:
				case ExpressionOp::LessThan:
				case ExpressionOp::GreaterThan:
				case ExpressionOp::LessThanOrEqual:
				case ExpressionOp::GreaterThanOrEqual:
				case ExpressionOp::LogicalAnd:
				case ExpressionOp::LogicalOr:
				case ExpressionOp::BitwiseAnd:
				case ExpressionOp::BitwiseOr:
				case ExpressionOp::BitwiseXor:
				case ExpressionOp::BitwiseInvert:
				case ExpressionOp::BitwiseLeftShift:
				case ExpressionOp::BitwiseRightShift:
					return ExpressionType::EXPR_NUMBER;

				case ExpressionOp::Assign:
					return right->expressionType();

				case ExpressionOp::Concat:
					return ExpressionType::EXPR_STRING;
			}

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

		virtual ExpressionType expressionType() const
		{
			switch (op)
			{
				case ExpressionOp::UnaryMinus:
				case ExpressionOp::UnaryNot:
					return ExpressionType::EXPR_NUMBER;
			}

			return expr->expressionType();
		}
};

class ExpressionFnCallNode : public ExpressionNode
{
public:
	_NodeName("ExpressionFnCallNode")

	ExpressionFnCallNode(ExpressionNode *funcExpr, ExpressionNode *objExpr, std::vector<ExpressionNode *> *argList = nullptr)
		: ExpressionNode(), funcExpr(funcExpr), objExpr(objExpr)
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

	virtual std::string toString() const {
		std::string str("new");
		for (const auto& dim : dimensions)
			str.append("[").append(std::to_string(dim)).append("]");

		return str;
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_ARRAY;
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

	virtual std::string toString() const {
		std::string str = "new ";
		return str + newExpr->toString();
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_OBJECT;
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

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_ARRAY;
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

	ExpressionNode *expr;
	StatementNode *thenBlock;
	StatementNode *elseBlock;
};

class StatementFnDeclNode : public StatementNode
{
public:
	_NodeName("StatementFnDeclNode")

	StatementFnDeclNode(std::string *id, std::vector<ExpressionNode *> *argList, StatementBlock *block, std::string *objName = nullptr)
		: StatementNode(), stmtBlock(block), pub(false), emit_prejump(true), ident(id), objectName(objName)
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

	void setPublic(bool r) {
		pub = r;
	}

	bool pub;
	bool emit_prejump;
	std::string *ident, *objectName;
	StatementBlock *stmtBlock;
	std::vector<ExpressionNode *> args;
};

class StatementNewNode : public StatementNode
{
public:
	_NodeName("StatementNewNode")

	StatementNewNode(std::string *objName, std::vector<ExpressionNode *> *argList, StatementBlock *block)
		: StatementNode(), stmtBlock(block), ident(objName)
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
	
	std::string *ident;
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

	ExpressionNode *expr;
	StatementNode *block;
};

class StatementForNode : public StatementNode
{
public:
	_NodeName("StatementForNode")

	StatementForNode(ExpressionNode *init, ExpressionNode *cond, ExpressionNode *incr, StatementNode *block);

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

	ExpressionNode *name;
	ExpressionNode *expr;
	StatementNode *block;
};


class ExpressionFnObject : public ExpressionNode
{
public:
	_NodeName("ExpressionFnObject")

	ExpressionFnObject(std::string *id, std::vector<ExpressionNode *> *argList, StatementBlock* block)
		: ExpressionNode(), ident(id), fnNode(id, argList, block)
	{
		takeOwnership(&fnNode);

		fnNode.emit_prejump = false;
		fnNode.setPublic(true);
	}

	virtual ExpressionType expressionType() const {
		return ExpressionType::EXPR_FUNCTIONOBJ;
	}

	virtual std::string toString() const {
		return "() -> { }";
	}

	std::string *ident;
	StatementFnDeclNode fnNode;
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

	ExpressionNode *expr;
	std::vector<SwitchCaseState> cases;
};

struct EnumMember
{
	std::string *node;
	bool hasIndex;
	int idx;

	EnumMember(std::string *n)
		: node(n), idx(0), hasIndex(false)
	{

	}

	EnumMember(std::string *n, int idx)
		: node(n), idx(idx), hasIndex(true)
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

namespace ast
{
	/**
	 * Checks if the postfix node has only one child, and if that is the case
	 * it will return the child otherwise it will return the postfix node back
	 */
	ExpressionNode * checkPostfixNode(ExpressionPostfixNode *node);
}

#endif
