#include <cassert>
#include "Parser.h"
#include "ast.h"

#include "calc.tab.hh"
#include "lex.yy.h"

ParserData::ParserData()
	: lineNumber(0), prog(nullptr), buffer(nullptr), newObjCallCount(0)
{
	yylex_init_extra(this, &scanner);
}

ParserData::~ParserData()
{
	if (buffer)
		yy_delete_buffer(buffer, scanner);

	yylex_destroy(scanner);
	delete prog;
}

const char * ParserData::saveString(const char* str, int length)
{
	auto ins = stable.insert(std::string(str, length));
	return ins.first->c_str();
}

void ParserData::addEnum(EnumList *enumList, const std::string& name)
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
	if (buffer)
		yy_delete_buffer(buffer, scanner);

	if (prog)
	{
		delete prog;
		prog = nullptr;
	}

	lineNumber = 1;
	buffer = yy_scan_string(input.c_str(), scanner);
	yyparse(this, scanner);
}
