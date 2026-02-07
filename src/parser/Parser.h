#pragma once

#ifndef MYPARSER_H
#define MYPARSER_H

#include <memory>
#include <optional>
#include <string>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <format>
#include "ast/ast.h"
#include "memory/ArenaAllocator.h"
#include "exceptions/GS2CompilerError.h"

typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;

class ParserContext
{
	public:
		ParserContext(GS2ErrorService& service);
		ParserContext(ParserContext&& o) noexcept = delete;
		ParserContext(const ParserContext&) = delete;
		~ParserContext();

		ParserContext& operator=(const ParserContext&) = delete;

		/**
		 * Returns the error service to push errors encountered during the
		 * parser/compilation process.
		 *
		 * @return ErrorService
		 */
		auto & getErrorService() {
			return errorService;
		}

		/**
		 * Parse an input into the parse context, and create an
		 * abstract syntax tree retrievable via getRootStatement() upon
		 * success
		 * @param source
		 *
		 * @return true if success, false otherwise
		 */
		bool parse(const std::string& source);

		/**
		 * Pushes a compile error to the error service
		 * 
		 * @param GS2CompilerError
		 */
		void addError(GS2CompilerError error);

		/**
		 * Creates a new compiler error from the string, and includes the line number
		 * and line contents in the error string
		 * 
		 * @param string error_msg
		 */
		void addParserError(const std::string& errmsg);

	// Accessed by bison
	public:
		int lineNumber;
		int columnNumber;

		std::string * saveString(const char* str, int length, bool unquote = false);
		std::string * generateLambdaFuncName();

		/*
		 * Add/get constants - used by bison during parsing
		 */
		void addConstant(const std::string& ident, ExpressionIdentifierNode *node);
		void addConstant(const std::string& ident, ExpressionNode *node);
		ExpressionNode * getConstant(const std::string& key) const;

		/*
		 * Add the enum to the current constant space, used by bison when parsing
		 */
		void addEnum(EnumList *enumList, std::string prefix = "");

		/*
		 * Switch-Case Statements:
		 * Helps parse the switch-case statements into a proper
		 * data structure that we can easily traverse at a later time.
		 */
		SwitchCaseState popCaseExpr();
		void pushCaseExpr(ExpressionNode *expr);
		void setCaseStatement(StatementBlock *block);

		/*
		 * A pointer to the root node of the abstract syntax tree
		 */
		StatementBlock * getRootStatement() const;

		/*
		 * Used by bison to set the root statement from the parser
		 * ** Should not be used normally
		 */
		void setRootStatement(StatementBlock *block);

		/*
		 * Allocates a node for the parser, the memory is managed
		 * by the parser context
		 */
		template<typename T, typename... P>
		T *alloc(P&&... params);

	private:
		/**
		 * Cleanup any nodes allocated
		 */
		void cleanup();

		/**
		 * Reset parser state
		 */
		void reset();

	private:
		yyscan_t scanner;
		YY_BUFFER_STATE buffer;

		bool failed;
		const std::string *inputStringPtr;
		size_t lambdaFunctionCount;
		std::unordered_map<std::string, ExpressionNode *> constantsTable;
		std::unordered_map<std::string, std::shared_ptr<std::string>> stringTable;
		std::stack<SwitchCaseState> switchCases;

		ArenaAllocator<> nodeArena;
		StatementBlock* programNode;
		GS2ErrorService& errorService;
};

/*
 * Constants Table
 */
inline ExpressionNode * ParserContext::getConstant(const std::string& key) const
{
	auto it = constantsTable.find(key);
	if (it == constantsTable.end())
		return nullptr;

	return it->second;
}

/*
 * Switch-Case Statements
 */
inline void ParserContext::pushCaseExpr(ExpressionNode* expr)
{
	switchCases.top().exprList.push_back(expr);
}

inline void ParserContext::setCaseStatement(StatementBlock* block)
{
	switchCases.push(SwitchCaseState{ block });
}

inline SwitchCaseState ParserContext::popCaseExpr()
{
	SwitchCaseState state = std::move(switchCases.top());
	switchCases.pop();
	return state;
}

/*
 * Abstract Syntax Tree Node - set by bison
 */
inline StatementBlock * ParserContext::getRootStatement() const
{
	return programNode;
}

inline void ParserContext::setRootStatement(StatementBlock *block)
{
	programNode = block;
}

/*
 * Push errors to the error service
 */
inline void ParserContext::addError(GS2CompilerError error)
{
	if (error.level() == ErrorLevel::E_ERROR)
		failed = true;

	getErrorService().submitPayload( std::move(error));
}

/*
 * Memory Allocation for Nodes
 *
 * Uses arena allocator for fast allocation with excellent cache locality.
 * All nodes are freed together when the ParserContext is destroyed or reset.
 */
template<typename T, typename ...P>
inline T *ParserContext::alloc(P && ...params)
{
	T *n = nodeArena.allocate<T>(std::forward<P>(params)...);
	return n;
}

#endif
