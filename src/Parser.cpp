#include <cassert>
#include "Parser.h"
#include "ast.h"

#include "calc.tab.hh"
#include "lex.yy.h"

ParserData::ParserData()
	: lineNumber(0), prog(nullptr)
{
	yylex_init_extra(this, &scanner);
}

ParserData::~ParserData()
{
	yylex_destroy(scanner);
	delete prog;
}

const char * ParserData::saveString(const char* str, int length)
{
	assert(length > 0);
	auto ins = stable.insert(std::string(str, length));
	return ins.first->c_str();
}

void ParserData::addEnum(EnumList *enumList)
{
	for (const auto& en : enumList->getMembers())
		enumConstants[en->node] = en->idx;
	delete enumList;
}

std::optional<int> ParserData::getEnumConstant(const std::string& key)
{
	auto it = enumConstants.find(key);
	if (it == enumConstants.end())
		return {};

	return it->second;
}

void ParserData::parse(const std::string& input)
{
	yy_switch_to_buffer(yy_scan_string(input.c_str(), scanner), scanner);
	yyparse(this, scanner);
}
