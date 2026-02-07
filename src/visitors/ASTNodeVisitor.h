#pragma once

#ifndef ASTVISITORIMPL_H
#define ASTVISITORIMPL_H

#include "../ast/ast.h"

class ASTNodeVisitor : public NodeVisitor
{
public:
	virtual void Visit(Node *node) {}
	virtual void Visit(StatementNode *node) {}
	virtual void Visit(ExpressionNode *node) {}
	virtual void Visit(StatementBreakNode *node) {}
	virtual void Visit(StatementContinueNode *node) {}
	virtual void Visit(ExpressionIdentifierNode *node) {}
	virtual void Visit(ExpressionStringConstNode *node) {}
	virtual void Visit(ExpressionIntegerNode *node) {}
	virtual void Visit(ExpressionNumberNode *node) {}
	virtual void Visit(ExpressionConstantNode *node) {}
	virtual void Visit(ExpressionNewArrayNode *node) {}

	virtual void Visit(StatementBlock* node)
	{
		for (const auto& n : node->statements)
			n->visit(this);
	}

	virtual void Visit(StatementIfNode *node)
	{
		node->expr->visit(this);
		node->thenBlock->visit(this);

		if (node->elseBlock)
			node->elseBlock->visit(this);
	}

	virtual void Visit(StatementFnDeclNode *node)
	{
		node->stmtBlock->visit(this);
	}

	virtual void Visit(StatementNewNode *node)
	{
		for (const auto& n : node->args)
			n->visit(this);

		if (node->stmtBlock)
			node->stmtBlock->visit(this);
	}

	virtual void Visit(StatementReturnNode *node)
	{
		if (node->expr)
			node->expr->visit(this);
	}

	virtual void Visit(StatementForNode *node)
	{
		if (node->init)
			node->init->visit(this);

		if (node->cond)
			node->cond->visit(this);

		if (node->block)
			node->block->visit(this);

		if (node->postop)
			node->postop->visit(this);
	}

	virtual void Visit(StatementForEachNode *node)
	{
		node->name->visit(this);
		node->expr->visit(this);

		node->block->visit(this);
	}

	virtual void Visit(StatementSwitchNode *node)
	{
		for (const auto& caseNode : node->cases)
			caseNode.block->visit(this);

		node->expr->visit(this);

		for (const auto &caseNode : node->cases)
		{
			for (const auto &caseExpr : caseNode.exprList)
			{
				if (caseExpr)
					caseExpr->visit(this);
			}
		}
	}

	virtual void Visit(StatementWhileNode *node)
	{
		node->expr->visit(this);
		node->block->visit(this);
	}

	virtual void Visit(StatementWithNode *node)
	{
		node->expr->visit(this);

		if (node->block)
			node->block->visit(this);
	}

	virtual void Visit(ExpressionPostfixNode *node)
	{
		for (const auto& n : node->nodes)
			n->visit(this);
	}
	
	virtual void Visit(ExpressionInOpNode *node)
	{
		node->expr->visit(this);
		node->lower->visit(this);

		if (node->higher)
			node->higher->visit(this);
	}

	virtual void Visit(ExpressionCastNode *node)
	{
		node->expr->visit(this);
	}

	virtual void Visit(ExpressionArrayIndexNode *node)
	{
		for (const auto& expr : node->exprList)
			expr->visit(this);
	}

	virtual void Visit(ExpressionFnCallNode *node)
	{
		if (node->objExpr)
			node->objExpr->visit(this);

		for (const auto& n : node->args)
			n->visit(this);

		node->funcExpr->visit(this);
	}

	virtual void Visit(ExpressionNewObjectNode *node)
	{
		// new only works with one argument, and the argument is the object name
		if (node->args.size() == 1)
			node->args.front()->visit(this);
	}

	virtual void Visit(ExpressionTernaryOpNode *node)
	{
		node->condition->visit(this);
		node->leftExpr->visit(this);
		node->rightExpr->visit(this);
	}

	virtual void Visit(ExpressionBinaryOpNode *node)
	{
		node->left->visit(this);
		node->right->visit(this);
	}

	virtual void Visit(ExpressionUnaryOpNode *node)
	{
		node->expr->visit(this);
	}

	virtual void Visit(ExpressionStrConcatNode *node)
	{
		node->left->visit(this);
		node->right->visit(this);
	}

	virtual void Visit(ExpressionListNode *node)
	{
		for (auto it = node->args.rbegin(); it != node->args.rend(); ++it)
			(*it)->visit(this);
	}

	virtual void Visit(ExpressionFnObject *node)
	{
		Visit(&node->fnNode);
	}
};

#endif
