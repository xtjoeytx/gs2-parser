#include "ast.h"

#ifdef DBGALLOCATIONS
#include <mutex>

int alloc_count = 0;
int total_alloc_count = 0;
std::mutex astCountLock;
std::vector<Node*> n;

void checkForNodeLeaks()
{
	std::scoped_lock lock(astCountLock);

	for (const auto& v : n)
		printf("Nodes alive: %s\n", v->NodeType());
	printf("Nodes leaked: %d / %d\n", alloc_count, total_alloc_count);
}

void checkNodeOwnership()
{
	for (const auto& v : n)
	{
		if (v->parent == nullptr)
		{
			printf("Nodes without parent: %s\n", v->NodeType());

			if (strcmp(v->NodeType(), ExpressionPostfixNode::NodeName) == 0)
			{
				auto n = reinterpret_cast<ExpressionPostfixNode*>(v);
				printf("	Postfix: %s\n", n->toString().c_str());
			}
		}
	}
}
#endif

Node::Node()
	: parent(nullptr)
{
#ifdef DBGALLOCATIONS
	{
		std::scoped_lock lock(astCountLock);
		n.push_back(this);
		alloc_count++;
		total_alloc_count++;
		//printf("Nodes in memory: %d\n", alloc_count);
	}
#endif
}

Node::~Node()
{
#ifdef DBGALLOCATIONS
	{
		std::scoped_lock lock(astCountLock);
		n.erase(std::remove(n.begin(), n.end(), this), n.end());
		--alloc_count;
		//printf("Nodes in memory: %d\n", alloc_count);
	}
#endif
}

StatementBlock::~StatementBlock()
{
	//for (const auto& node : statements)
	//	delete node;
	//statements.clear();
}

void StatementBlock::append(StatementNode *node)
{
	if (node)
	{
		//
		node->parent = this;

		// TODO(joey): come back to this
		if (strcmp(node->NodeType(), ExpressionPostfixNode::NodeName) == 0)
		{
			auto pfNode = reinterpret_cast<ExpressionPostfixNode*>(node);
			if (pfNode->expressionType() == ExpressionType::EXPR_FUNCTION)
			{
				auto fnNode = reinterpret_cast<ExpressionFnCallNode*>(pfNode->lastNode());
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
	//delete expr;
	//delete thenBlock;
	//delete elseBlock;
}

StatementFnDeclNode::~StatementFnDeclNode()
{
	//delete stmtBlock;

	//for (auto& node : args)
	//	delete node;
}

StatementNewNode::~StatementNewNode()
{
	//delete stmtBlock;

	//for (const auto& node : args)
	//	delete node;
}

StatementReturnNode::~StatementReturnNode()
{
	//delete expr;
}

StatementWhileNode::~StatementWhileNode()
{
	//delete expr;
	//delete block;
}

StatementWithNode::~StatementWithNode()
{
	//delete expr;
	//delete block;
}

StatementForNode::~StatementForNode()
{
	//delete init;
	//delete cond;
	//delete postop;
	//delete block;
}

StatementForEachNode::~StatementForEachNode()
{
	//delete name;
	//delete expr;
	//delete block;
}

StatementSwitchNode::~StatementSwitchNode()
{
	//for (const auto& caseList : cases)
	//{
		//for (const auto& matchExpr : caseList.exprList)
		//{
		//	// default case is nullptr
		//	if (matchExpr)
		//		delete matchExpr;
		//}

		//delete caseList.block;
	//}
	
	//delete expr;
}

ExpressionCastNode::~ExpressionCastNode()
{
	//delete expr;
}

ExpressionFnCallNode::~ExpressionFnCallNode()
{
	//for (const auto& node : args)
	//	delete node;
	//delete funcExpr;
	//delete objExpr;
}

ExpressionListNode::~ExpressionListNode()
{
	//for (const auto& node : args)
	//	delete node;
}

ExpressionNewObjectNode::~ExpressionNewObjectNode()
{
	//for (const auto& node : args)
	//	delete node;
	//delete newExpr;
}

ExpressionPostfixNode::~ExpressionPostfixNode()
{
	//for (const auto& node : nodes)
	//	delete node;
}

ExpressionArrayIndexNode::~ExpressionArrayIndexNode()
{
	//for (const auto& expr : exprList)
	//	delete expr;
}

ExpressionBinaryOpNode::~ExpressionBinaryOpNode()
{
	//delete left;
	//delete right;
}

ExpressionUnaryOpNode::~ExpressionUnaryOpNode()
{
	//delete expr;
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
