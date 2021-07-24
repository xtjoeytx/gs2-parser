#pragma once

#ifndef MYPARSER_H
#define MYPARSER_H

#include <optional>
#include <string>
#include <set>
#include <stack>
#include <unordered_map>
#include <vector>
#include "ast.h"

typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;

class ParserData
{
	public:
		ParserData();
		~ParserData();

		const char * saveString(const char *str, int length);

		// enums
		void addEnum(EnumList *enumList, const std::string& name = "");
		std::optional<int> getEnumConstant(const std::string& key);
		
		// switch case-expressions
		SwitchCaseState popCaseExpr();
		void pushCaseExpr(ExpressionNode* expr);
		void setCaseStatement(StatementBlock* block);

		//
		void parse(const std::string& input);
		
		StatementBlock *prog;
		int lineNumber;

	private:
		yyscan_t scanner;
		YY_BUFFER_STATE buffer;

		std::unordered_map<std::string, int> enumConstants;
		std::set<std::string> stable;
		std::stack<SwitchCaseState> switchCases;
};

inline void ParserData::pushCaseExpr(ExpressionNode* expr)
{
	switchCases.top().exprList.push_back(expr);
}

inline void ParserData::setCaseStatement(StatementBlock* block)
{
	switchCases.push(SwitchCaseState{ block });
}

inline SwitchCaseState ParserData::popCaseExpr()
{
	SwitchCaseState state = std::move(switchCases.top());
	switchCases.pop();
	return state;
}

#endif
