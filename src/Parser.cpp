#include <cassert>
#include "Parser.h"

#include "gs2parser.tab.hh"
#include "lex.yy.h"

void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

ParserContext::ParserContext()
	: lineNumber(0), columnNumber(0), buffer(nullptr), programNode(nullptr), state(ParserState::START)
{
	yylex_init_extra(this, &scanner);
}

// ParserData::ParserData(ParserData&& o) noexcept
// {
// 	prog = o.prog;
// 	lineNumber = o.lineNumber;
// 	newObjCallCount = o.newObjCallCount;
// 	buffer = o.buffer;
// 	scanner = o.scanner;

// 	enumConstants = std::move(o.enumConstants);
// 	stable = std::move(o.stable);
// 	switchCases = std::move(o.switchCases);

// 	o.buffer = nullptr;
// 	o.scanner = nullptr;
// 	o.prog = nullptr;
// }

ParserContext::~ParserContext()
{
	if (scanner)
	{
		if (buffer)
			yy_delete_buffer(buffer, scanner);

		yylex_destroy(scanner);
	}

	cleanup();
}

void ParserContext::cleanup()
{
	for (auto& n : nodes)
		delete n;
	nodes.clear();
}

const char * ParserContext::saveString(const char* str, int length, bool unquote)
{
	auto tmpStr = std::string(str, length);
	if (unquote)
	{
		ReplaceStringInPlace(tmpStr, "\\\"", "\"");
		ReplaceStringInPlace(tmpStr, "\\n", "\n");
	}

	auto ins = stable.insert(std::move(tmpStr));
	return ins.first->c_str();
}

void ParserContext::addEnum(EnumList *enumList, std::string prefix)
{
	if (prefix.empty())
	{
		for (const auto& en : enumList->getMembers())
			enumConstants[en->node] = en->idx;
	}
	else
	{
		prefix.append("::");
		for (const auto& en : enumList->getMembers())
		{
			std::string key = prefix;
			key.append(en->node);

			enumConstants[key] = en->idx;
		}
	}

	delete enumList;
}

std::optional<int> ParserContext::getEnumConstant(const std::string& key)
{
	auto it = enumConstants.find(key);
	if (it == enumConstants.end())
		return {};

	return it->second;
}

void ParserContext::parse(const std::string& input)
{
	if (buffer)
		yy_delete_buffer(buffer, scanner);
	
	errorList.clear();
	lineNumber = 1;
	buffer = yy_scan_string(input.c_str(), scanner);
	yyparse(this, scanner);
}
