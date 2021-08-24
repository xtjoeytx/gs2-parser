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
	}
#endif
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
