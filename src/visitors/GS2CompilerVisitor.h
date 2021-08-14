#pragma once

#ifndef GS2COMPILER_H
#define GS2COMPILER_H

#include <string>
#include <stack>
#include <vector>
#include <unordered_map>
#include "ast/astvisitor.h"
#include "GS2Bytecode.h"
#include "GS2BuiltInFunctions.h"

struct LogicalBreakPoint
{
	uint32_t opbreak = 0;
	uint32_t opcontinue = 0;
	std::vector<size_t> breakPointLocs;
	std::vector<size_t> continuePointLocs;
};

class ParserContext;

class GS2CompilerVisitor : public NodeVisitor
{
	GS2Bytecode byteCode;
	ParserContext& parserContext;
	GS2BuiltInFunctions& builtIn;

	public:
		GS2CompilerVisitor(ParserContext& context, GS2BuiltInFunctions& builtin)
			: parserContext(context), copyAssignment(false), newObjectCount(0), builtIn(builtin)
		{
		}

		virtual ~GS2CompilerVisitor() = default;

		Buffer getByteCode() {
			return byteCode.getByteCode();
		}

		/////////

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

	private:
		bool copyAssignment;
		int newObjectCount;
		std::stack<LogicalBreakPoint> logicalBreakpoints;
		std::stack<LogicalBreakPoint> loopBreakpoints;

		void popBreakpoint(std::stack<LogicalBreakPoint>& bp);
		void addBreakLocation(std::stack<LogicalBreakPoint>& bp, size_t location);
		void addContinueLocation(std::stack<LogicalBreakPoint>& bp, size_t location);

		void pushLogicalBreakpoint(LogicalBreakPoint bp);
		void popLogicalBreakpoint();
		void addLogicalBreakLocation(size_t location);
		void addLogicalContinueLocation(size_t location);

		void pushLoopBreakpoint(LogicalBreakPoint bp);
		void popLoopBreakpoint();
		void addLoopBreakLocation(size_t location);
		void addLoopContinueLocation(size_t location);
};

inline void GS2CompilerVisitor::addBreakLocation(std::stack<LogicalBreakPoint>& bp, size_t location)
{
	if (!bp.empty())
		bp.top().breakPointLocs.push_back(location);
}

inline void GS2CompilerVisitor::addContinueLocation(std::stack<LogicalBreakPoint>& bp, size_t location)
{
	if (!bp.empty())
		bp.top().continuePointLocs.push_back(location);
}

/////////////

inline void GS2CompilerVisitor::pushLogicalBreakpoint(LogicalBreakPoint bp)
{
	logicalBreakpoints.push(bp);
}

inline void GS2CompilerVisitor::popLogicalBreakpoint()
{
	popBreakpoint(logicalBreakpoints);
}

inline void GS2CompilerVisitor::addLogicalBreakLocation(size_t location)
{
	addBreakLocation(logicalBreakpoints, location);
}

inline void GS2CompilerVisitor::addLogicalContinueLocation(size_t location)
{
	addContinueLocation(logicalBreakpoints, location);
}

/////////////

inline void GS2CompilerVisitor::pushLoopBreakpoint(LogicalBreakPoint bp)
{
	loopBreakpoints.push(bp);
}

inline void GS2CompilerVisitor::popLoopBreakpoint()
{
	popBreakpoint(loopBreakpoints);
}

inline void GS2CompilerVisitor::addLoopBreakLocation(size_t location)
{
	addBreakLocation(loopBreakpoints, location);
}

inline void GS2CompilerVisitor::addLoopContinueLocation(size_t location)
{
	addContinueLocation(loopBreakpoints, location);
}

#endif
