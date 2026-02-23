#pragma once

#ifndef GS2COMPILER_H
#define GS2COMPILER_H

#include <cassert>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include "ast/NodeVisitor.h"
#include "GS2Bytecode.h"

class ParserContext;

class JumpTarget
{
	using jmp_address = uint32_t;

	GS2Bytecode& byteCode;
	std::vector<size_t> pending;
	std::optional<jmp_address> addr;

public:
	explicit JumpTarget(GS2Bytecode& bc) : byteCode(bc) {}

	~JumpTarget()
	{
		assert(pending.empty() && "unresolved forward references — missing resolve() call");
	}

	JumpTarget(const JumpTarget&) = delete;
	JumpTarget& operator=(const JumpTarget&) = delete;
	JumpTarget(JumpTarget&&) = delete;
	JumpTarget& operator=(JumpTarget&&) = delete;

	void addRef(size_t pos)
	{
		if (addr)
			byteCode.emit(short(*addr), pos);
		else
			pending.push_back(pos);
	}

	void resolve(jmp_address target)
	{
		addr = target;
		for (auto pos : pending)
			byteCode.emit(short(*addr), pos);
		pending.clear();
	}

	void resolveHere() { resolve(byteCode.getOpIndex()); }

	jmp_address address() const { return *addr; }

	size_t pendingCount() const { return pending.size(); }

	void reset()
	{
		assert(pending.empty());
		addr.reset();
	}

	void emitJump(opcode::Opcode jumpOp)
	{
		byteCode.emit(jumpOp);
		byteCode.emit(char(0xF4));
		byteCode.emit(short(0));
		addRef(byteCode.getBytecodePos() - 2);
	}
};

class GS2CompilerVisitor : public NodeVisitor
{
	using jmp_address = uint32_t;

	public:
		GS2CompilerVisitor(ParserContext& context);

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
		std::set<std::string> joinedClasses;

		bool _isCopyAssignment = false;
		bool _isInlineConditional = true;
		bool _isInsideExpression = false;
		int _newObjectCount = 0;

		// Jump targets
		JumpTarget* success_target = nullptr;
		JumpTarget* fail_target = nullptr;
		JumpTarget* break_target = nullptr;
		JumpTarget* continue_target = nullptr;
		JumpTarget* fn_skip_target = nullptr;
		bool _isRootBlock = true;

		struct ScopeGuard
		{
			GS2CompilerVisitor& self;
			JumpTarget* saved_success;
			JumpTarget* saved_fail;
			JumpTarget* saved_break;
			JumpTarget* saved_continue;

			explicit ScopeGuard(GS2CompilerVisitor& s)
				: self(s),
				  saved_success(s.success_target),
				  saved_fail(s.fail_target),
				  saved_break(s.break_target),
				  saved_continue(s.continue_target) {}

			~ScopeGuard()
			{
				self.success_target = saved_success;
				self.fail_target = saved_fail;
				self.break_target = saved_break;
				self.continue_target = saved_continue;
			}

			ScopeGuard(const ScopeGuard&) = delete;
			ScopeGuard& operator=(const ScopeGuard&) = delete;
		};
};

inline Buffer GS2CompilerVisitor::getByteCode()
{
	return byteCode.getByteCode();
}

inline const std::set<std::string>& GS2CompilerVisitor::getJoinedClasses() const
{
	return joinedClasses;
}

#endif
