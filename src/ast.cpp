#include "ast.h"

#ifdef DBGALLOCATIONS
int alloc_count = 0;
std::vector<Node*> n;

void checkForNodeLeaks()
{
	for (const auto& v : n) {
		printf("Nodes alive: %s\n", v->NodeType());
	}
}
#endif

Node::Node() {
#ifdef DBGALLOCATIONS
	n.push_back(this);
	printf("Count: %d\n", ++alloc_count);
#endif
}

Node::~Node() {
#ifdef DBGALLOCATIONS
	n.erase(std::remove(n.begin(), n.end(), this), n.end());
	printf("Count: %d\n", --alloc_count);
#endif
}

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

void StatementBlock::append(StatementNode *node)
{
	if (node)
	{
		// TODO(joey): come back to this
		if (strcmp(node->NodeType(), ExpressionPostfixNode::NodeName) == 0)
		{
			auto pfNode = reinterpret_cast<ExpressionPostfixNode*>(node);
			if (pfNode->expressionType() == ExpressionType::EXPR_FUNCTION)
			{
				auto fnNode = reinterpret_cast<ExpressionFnCallNode*>(pfNode->nodes.back());
				fnNode->discardReturnValue = true;
			}
		}
		else if (strcmp(node->NodeType(), ExpressionUnaryOpNode::NodeName) == 0)
		{
			// doesn't utilize the value, so we emit the operator the same way
			// we do operator-first unary ops. involves just a single inc operator
			// rather than pushing the node back to the stack
			auto unaryNode = reinterpret_cast<ExpressionUnaryOpNode *>(node);
			unaryNode->opFirst = true;
			unaryNode->opUnused = true;
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

	for (auto& node : args)
		delete node;
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
	for (const auto& node : cases)
		delete node;
	
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
	for (const auto& node : args)
		delete node;
}

ExpressionNewNode::~ExpressionNewNode()
{
	if (args)
	{
		for (const auto& node : *args)
			delete node;
	}
	delete newExpr;
}

ExpressionPostfixNode::~ExpressionPostfixNode()
{
	for (const auto& node : nodes)
		delete node;
}

ExpressionArrayIndexNode::~ExpressionArrayIndexNode()
{
	// NOTE: we are not the only ones with access to expr
	// should likely be removed from the constructor
	// delete expr;
	delete idx;
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

ExpressionInOpNode::~ExpressionInOpNode()
{
	delete expr;
	delete lower;
	delete higher;
}
