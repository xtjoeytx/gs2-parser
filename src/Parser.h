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
#include "ast/ast.h"
#include "exceptions/GS2CompilerError.h"

typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;

enum class ParserState {
	START,
	FUNC_INITIALIZER,
	STMT,
	END
};

class ParserContext
{
	public:
		ParserContext();
		ParserContext(ParserContext&& o) noexcept = delete;
		ParserContext(const ParserContext&) = delete;
		~ParserContext();

		ParserContext& operator=(const ParserContext&) = delete;

		//
		void parse(const std::string& input);

	// Accessed by bison
	public:
		void cleanup();

		std::string * saveString(const char* str, int length, bool unquote = false);
		std::string * generateLambdaFuncName();

		// constants
		void addConstant(const std::string& ident, ExpressionIdentifierNode *node);
		void addConstant(const std::string& ident, ExpressionNode *node);
		ExpressionNode * getConstant(const std::string& key);
		
		// enums
		void addEnum(EnumList *enumList, std::string prefix = "");
		
		// switch case-expressions
		SwitchCaseState popCaseExpr();
		void pushCaseExpr(ExpressionNode *expr);
		void setCaseStatement(StatementBlock *block);

		StatementBlock * getRootStatement() const;
		void setRootStatement(StatementBlock *block);

		// error handling
		bool hasErrors() const;
		std::vector<GS2CompilerError>& getErrorList();
		void addError(GS2CompilerError error);

		// accessed directly by bison
		int lineNumber;
		int columnNumber;
		ParserState state;

		template<typename T, typename... P>
		T* alloc(P&&... params)
		{
			T* n = new T(std::forward<P>(params)...);
			nodes.push_back(n);
			return n;
		}

		template<typename T>
		void dealloc(T* n)
		{
			if (n)
			{
				nodes.erase(std::remove(nodes.begin(), nodes.end(), n), nodes.end());
				delete n;
			}
		}

	private:
		yyscan_t scanner;
		YY_BUFFER_STATE buffer;

		size_t lambdaFunctionCount;
		std::unordered_map<std::string, ExpressionNode *> constantsTable;
		std::unordered_map<std::string, std::shared_ptr<std::string>> stringTable;
		std::stack<SwitchCaseState> switchCases;
		std::vector<GS2CompilerError> errorList;

		std::vector<Node*> nodes;
		StatementBlock* programNode;
};

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

inline StatementBlock * ParserContext::getRootStatement() const
{
	return programNode;
}

inline void ParserContext::setRootStatement(StatementBlock *block)
{
	programNode = block;
}


// error handling

inline bool ParserContext::hasErrors() const
{
	return !errorList.empty();
}

inline std::vector<GS2CompilerError>& ParserContext::getErrorList()
{
	return errorList;
}

inline void ParserContext::addError(GS2CompilerError error)
{
	errorList.push_back(std::move(error));
}

#endif
