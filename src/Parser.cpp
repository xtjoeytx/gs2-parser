#include <cassert>
#include "Parser.h"

#include "gs2parser.tab.hh"
#include "lex.yy.h"

std::string GetLineByLineNumber(const std::string& subject, uint32_t lineNumber)
{
	size_t pos = 0;
	for (uint32_t i = 0; i < lineNumber - 1; i++)
	{
		pos = subject.find("\n", pos);
		if (pos == std::string::npos)
			return {};

		++pos;
	}

	auto end = subject.find("\n", pos);
	if (end == std::string::npos)
		return subject.substr(pos);
	return subject.substr(pos, end - pos);
}

void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

ParserContext::ParserContext(GS2ErrorService& service)
		: lineNumber(0), columnNumber(0), buffer(nullptr), failed(false), inputStringPtr(nullptr),
		  lambdaFunctionCount(0), programNode(nullptr), errorService(service)
{
	yylex_init_extra(this, &scanner);
}

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

void ParserContext::reset()
{
	// Cleanup any allocated nodes
	cleanup();

	// Reset our tables
	constantsTable = {};
	switchCases = {};
	stringTable = {};

	// Delete the buffer associated with the parser
	if (buffer)
	{
		yy_delete_buffer(buffer, scanner);
		buffer = nullptr;
	}

	lineNumber = 1;
	columnNumber = 0;
	programNode = nullptr;
	inputStringPtr = nullptr;
	lambdaFunctionCount = 0;
	failed = false;
}

std::string* ParserContext::saveString(const char* str, int length, bool unquote)
{
	auto tmpStr = std::string(str, length);
	if (unquote)
	{
		ReplaceStringInPlace(tmpStr, "\\\"", "\"");
		ReplaceStringInPlace(tmpStr, "\\n", "\n");
	}

	auto it = stringTable.find(tmpStr);
	if (it != stringTable.end())
		return it->second.get();

	auto ptr = std::make_shared<std::string>(std::move(tmpStr));
	auto ret = stringTable.insert({ *ptr, ptr });
	return ptr.get(); // warning: C26816, but seems ok to me
	//return ret.first->second.get();
}

std::string * ParserContext::generateLambdaFuncName()
{
	std::string fnName = fmt::format("function_{}_1", 100 + lambdaFunctionCount);
	lambdaFunctionCount++;
	return saveString(fnName.c_str(), int(fnName.length()));
}

void ParserContext::addEnum(EnumList *enumList, std::string prefix)
{
	if (prefix.empty())
	{
		for (const auto& en : enumList->getMembers())
			addConstant(*en->node, alloc<ExpressionIntegerNode>(en->idx));
	}
	else
	{
		prefix.append("::");
		for (const auto& en : enumList->getMembers())
		{
			std::string key = prefix;
			key.append(*en->node);

			addConstant(key, alloc<ExpressionIntegerNode>(en->idx));
		}
	}

	delete enumList;
}

void ParserContext::addConstant(const std::string& ident, ExpressionIdentifierNode *node)
{
	auto constant = getConstant(ident);
	if (constant)
	{
		// report error - redefining constant
		addParserError(fmt::format("redefinition of constant {}", ident));
		return;
	}

	ExpressionNode *constNode;
	
	// Map ident to the same constant pointed from the right hand identifier
	auto rhIdent = node->toString();
	constNode = getConstant(rhIdent);

	// Handle special constants that we can't define directly as a constant
	// because gs2 allows these identifiers in object field names.
	if (!constNode)
	{
		if (rhIdent == "true")
		{
			constNode = alloc<ExpressionConstantNode>(ExpressionConstantNode::ConstantType::TRUE_T);
		}
		else if (rhIdent == "false")
		{
			constNode = alloc<ExpressionConstantNode>(ExpressionConstantNode::ConstantType::FALSE_T);
		}
		else if (rhIdent == "null")
		{
			constNode = alloc<ExpressionConstantNode>(ExpressionConstantNode::ConstantType::NULL_T);
		}
		else
		{
			// report error - constant does not exist
			addParserError(fmt::format("constant {} is undefined", ident));
			return;
		}
	}
	
	constantsTable[ident] = constNode;
}

void ParserContext::addConstant(const std::string& ident, ExpressionNode *node)
{
	if (node->expressionType() == ExpressionType::EXPR_IDENT)
	{
		addConstant(ident, (ExpressionIdentifierNode *)node);
		return;
	}

	auto constant = getConstant(ident);
	if (constant)
	{
		addParserError(fmt::format("redefinition of constant {}", ident));
		return;
	}

	constantsTable[ident] = node;
}

void ParserContext::addParserError(const std::string& errmsg)
{
	assert(inputStringPtr != nullptr);

	std::string lineText;
	if (inputStringPtr)
		lineText = GetLineByLineNumber(*inputStringPtr, lineNumber);

	std::string msg;
	if (lineText.empty())
	{
		msg = fmt::format("parser error occurred near line {}: {}", lineNumber, errmsg);
	}
	else
	{
		msg = fmt::format("{} at line {}: {}", errmsg, lineNumber, lineText);
	}
	
	addError({ ErrorLevel::E_ERROR, GS2CompilerError::ErrorCategory::Parser, std::move(msg) });
}

bool ParserContext::parse(const std::string& source)
{
	reset();

	// Holding a pointer to the source incase we have an error msg raised
	inputStringPtr = &source;
	buffer = yy_scan_string(source.c_str(), scanner);
	yyparse(this, scanner);
	return !failed;
}
