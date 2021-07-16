#include "ast.h"

ProgramNode::~ProgramNode()
{
	for (const auto& node : nodes)
		delete node;
	nodes.clear();
}

StatementBlock::~StatementBlock()
{
	for (const auto& node : statements)
		delete node;
	statements.clear();
}

void StatementBlock::append(StatementNode *node) {
	if (node)
	{
		if (strcmp(node->NodeType(), ExpressionFnCallNode::NodeName) == 0)
		{
			auto fnNode = reinterpret_cast<ExpressionFnCallNode*>(node);
			fnNode->discardReturnValue = true;
		}

		statements.push_back(node);
	}
}

StatementIfNode::~StatementIfNode()
{
	delete expr;
	delete thenBlock;
	delete elseBlock;
}

StatementFnDeclNode::~StatementFnDeclNode()
{
	delete stmtBlock;
	if (args)
	{
		for (const auto& node : *args)
			delete node;
		args->clear();
		args = nullptr;
	}
}

StatementNewNode::~StatementNewNode()
{
	delete stmtBlock;
	if (args)
	{
		for (const auto& node : *args)
			delete node;
	}
}

StatementReturnNode::~StatementReturnNode()
{
	delete expr;
}

StatementWhileNode::~StatementWhileNode()
{
	delete expr;
	delete block;
}

StatementWithNode::~StatementWithNode()
{
	delete expr;
	delete block;
}

StatementForNode::~StatementForNode()
{
	delete init;
	delete cond;
	delete postop;
	delete block;
}

StatementForEachNode::~StatementForEachNode()
{
	delete name;
	delete expr;
	delete block;
}

StatementSwitchNode::~StatementSwitchNode()
{
	if (cases)
	{
		for (const auto& node : *cases)
			delete node;
	}

	delete cases;
	delete expr;
}

CaseNode::~CaseNode()
{
	delete expr;
	delete stmt;
}

ExpressionCastNode::~ExpressionCastNode()
{
	delete expr;
}

ExpressionFnCallNode::~ExpressionFnCallNode()
{
	if (args)
	{
		for (const auto& node : *args)
			delete node;
	}
	delete funcExpr;
	delete objExpr;
}

ExpressionListNode::~ExpressionListNode()
{
	if (args)
	{
		for (const auto& node : *args)
			delete node;
	}
}

ExpressionObjectAccessNode::~ExpressionObjectAccessNode()
{
	delete left;
	delete right;

	for (const auto& node : nodes)
		delete node;
}

ExpressionBinaryOpNode::~ExpressionBinaryOpNode()
{
	delete left;
	delete right;
}

ExpressionUnaryOpNode::~ExpressionUnaryOpNode()
{
	delete expr;
}

void EnumList::addMember(EnumMember *member)
{
	if (!member->hasIndex)
	{
		member->hasIndex = true;
		member->idx = curIdx++;
	}
	else
	{
		curIdx = member->idx + 1;
	}

	members.push_back(member);
}