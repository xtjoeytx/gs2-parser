#pragma once

#ifndef GS2COMPILER_H
#define GS2COMPILER_H

#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include "ast/astvisitor.h"
#include "GS2Bytecode.h"
#include "utils/format_string.h"
#include "GS2BuiltInFunctions.h"

class ParserContext;

class GS2CompilerVisitor : public NodeVisitor
{
	using label_id = uint32_t;
	using jmp_address = uint32_t;

	public:
		GS2CompilerVisitor(ParserContext& context, GS2BuiltInFunctions& builtin);

		Buffer getByteCode();
		const std::set<std::string>& getJoinedClasses() const;

	public:
		virtual void Visit(Node *node);
		virtual void Visit(StatementNode *node);
		virtual void Visit(StatementBlock *node);
		virtual void Visit(StatementIfNode *node);
		virtual void Visit(StatementFnDeclNode *node);
		virtual void Visit(StatementNewNode *node);
		virtual void Visit(StatementBreakNode *node);
		virtual void Visit(StatementContinueNode *node);
		virtual void Visit(StatementReturnNode *node);
		virtual void Visit(StatementForNode *node);
		virtual void Visit(StatementForEachNode *node);
		virtual void Visit(StatementSwitchNode *node);
		virtual void Visit(StatementWhileNode *node);
		virtual void Visit(StatementWithNode *node);
		virtual void Visit(ExpressionNode *node);
		virtual void Visit(ExpressionIdentifierNode *node);
		virtual void Visit(ExpressionStringConstNode *node);
		virtual void Visit(ExpressionIntegerNode *node);
		virtual void Visit(ExpressionNumberNode *node);
		virtual void Visit(ExpressionPostfixNode *node);
		virtual void Visit(ExpressionCastNode *node);
		virtual void Visit(ExpressionArrayIndexNode *node);
		virtual void Visit(ExpressionInOpNode *node);
		virtual void Visit(ExpressionFnCallNode *node);
		virtual void Visit(ExpressionNewArrayNode *node);
		virtual void Visit(ExpressionNewObjectNode *node);
		virtual void Visit(ExpressionTernaryOpNode *node);
		virtual void Visit(ExpressionBinaryOpNode *node);
		virtual void Visit(ExpressionUnaryOpNode *node);
		virtual void Visit(ExpressionStrConcatNode *node);
		virtual void Visit(ExpressionListNode *node);
		virtual void Visit(ExpressionConstantNode *node);
		virtual void Visit(ExpressionFnObject* node);

	private:
		GS2Bytecode byteCode;
		ParserContext& parserContext;
		GS2BuiltInFunctions& builtIn;
		std::set<std::string> joinedClasses;

		bool _isCopyAssignment;
		bool _isInlineConditional;
		bool _isInsideExpression;
		int _newObjectCount;

		// Jump-labels
		label_id success_label, fail_label, exit_label;
		label_id break_label, continue_label;
		std::unordered_map<label_id, std::vector<size_t>> label_locs;
		std::unordered_map<label_id, jmp_address> label_addr;

		// Jump-label functions
		label_id createLabel();
		void addLocation(label_id label, size_t loc);
		void setLocation(label_id label, jmp_address addr);
		void writeLabels();
};

inline Buffer GS2CompilerVisitor::getByteCode()
{
	setLocation(exit_label, byteCode.getOpIndex());
	writeLabels();
	return byteCode.getByteCode();
}

inline const std::set<std::string>& GS2CompilerVisitor::getJoinedClasses() const
{
	return joinedClasses;
}

inline void GS2CompilerVisitor::addLocation(label_id label, size_t loc)
{
	label_locs[label].push_back(loc);
}

inline void GS2CompilerVisitor::setLocation(label_id label, jmp_address addr)
{
	label_addr[label] = addr;
}

#endif
