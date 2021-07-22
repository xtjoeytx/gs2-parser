#pragma once

#ifndef MYPARSER_H
#define MYPARSER_H

#include <optional>
#include <string>
#include <set>
#include <unordered_map>
#include <vector>
//#include "ast.h"

typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;

class StatementBlock;
class EnumList;

class ParserData
{
	public:
		ParserData();
		~ParserData();

		const char * saveString(const char *str, int length);

		void addEnum(EnumList *enumList, const std::string& name = "");
		std::optional<int> getEnumConstant(const std::string& key);
		
		void parse(const std::string& input);
		
		StatementBlock *prog;
		int lineNumber;

	private:
		yyscan_t scanner;
		YY_BUFFER_STATE buffer;

		std::unordered_map<std::string, int> enumConstants;
		std::set<std::string> stable;
};

#endif
