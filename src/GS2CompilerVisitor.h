#pragma once

#ifndef GS2COMPILER_H
#define GS2COMPILER_H

#include <stack>
#include "astvisitor.h"
#include "GS2Bytecode.h"

class ParserData;

struct LoopBreakPoint
{
	std::vector<size_t> continuePointLocs;
	std::vector<size_t> breakPointLocs;
};

struct LogicalBreakPoint
{
	size_t opbreak = 0;
	size_t opcontinue = 0;
	std::vector<size_t> breakPointLocs;
	std::vector<size_t> continuePointLocs;
};

class GS2CompilerVisitor : public NodeVisitor
{
	GS2Bytecode byteCode;
	ParserData *parserData;

	std::stack<LoopBreakPoint> breakPoints;
	std::stack<LogicalBreakPoint> logicalBreakpoints;

	public:
		GS2CompilerVisitor(ParserData *data) : parserData(data) { }
		virtual ~GS2CompilerVisitor() = default;

		void Reset() {
			byteCode.Reset();
		}

		Buffer getByteCode(const std::string& scriptType, const std::string& scriptName, bool saveToDisk = true) {
			return byteCode.getByteCode(scriptType, scriptName, saveToDisk);
		}

		void pushLogicalBreakpoint(LogicalBreakPoint bp)
		{
			logicalBreakpoints.push(bp);
		}

		void popLogicalBreakpoint()
		{
			auto& breakPoint = logicalBreakpoints.top();
			if (breakPoint.opbreak >= 0)
			{
				for (const auto& loc : breakPoint.breakPointLocs)
					byteCode.emit(short(breakPoint.opbreak), loc);
			}

			if (breakPoint.opcontinue >= 0)
			{
				for (const auto& loc : breakPoint.continuePointLocs)
					byteCode.emit(short(breakPoint.opcontinue), loc);
			}

			logicalBreakpoints.pop();
		}

		void addBreakLocation(size_t location)
		{
			if (!logicalBreakpoints.empty())
				logicalBreakpoints.top().breakPointLocs.push_back(location);
		}

		void addContinueLocation(size_t location)
		{
			if (!logicalBreakpoints.empty())
				logicalBreakpoints.top().continuePointLocs.push_back(location);
		}

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
};

#endif
