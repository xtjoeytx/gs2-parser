#pragma once

#ifndef MYPARSER_H
#define MYPARSER_H

#include <unordered_map>
#include <optional>
#include <string>
#include <set>
#include <vector>
#include "ast.h"

typedef void* yyscan_t;
class StatementBlock;
class EnumList;

class ParserData
{
	public:
		ParserData();
		~ParserData();

		const char * saveString(const char *s);

		void addEnum(EnumList *enumList);
		std::optional<int> getEnumConstant(const std::string& key);
		
		void parse(const std::string& input);
		
		StatementBlock *prog;
		int lineNumber;

	private:
		yyscan_t scanner;

		std::unordered_map<std::string, int> enumConstants;
		std::set<std::string> stable;
};

#endif