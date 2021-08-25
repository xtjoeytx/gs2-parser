#pragma once

#ifndef FUNCTIONINSPECTVISITOR_H
#define FUNCTIONINSPECTVISITOR_H

#include "ast/ast.h"
#include "ast/astnodevisitor.h"

class FunctionInspectVisitor : public ASTNodeVisitor
{
public:
	bool foundFunctionCall;

	FunctionInspectVisitor() : foundFunctionCall(false) { }

	virtual void Visit(ExpressionFnCallNode *node)
	{
		foundFunctionCall = true;

		ASTNodeVisitor::Visit(node);
	}

	template<class T>
	void Visit(T *node)
	{
		ASTNodeVisitor::Visit(node);
	}
};


#endif
