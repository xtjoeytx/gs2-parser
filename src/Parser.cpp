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
	: lineNumber(0), columnNumber(0), buffer(nullptr), programNode(nullptr), state(ParserState::START), lambdaFunctionCount(0)
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

std::string * ParserContext::saveString(const char* str, int length, bool unquote)
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
	std::string fnName;
	fnName.reserve(30);
	fnName.append("function_");
	fnName.append(std::to_string(420 + lambdaFunctionCount));
	fnName.append("_").append("1");
	
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
		if (rhIdent == "true") {
			constNode = alloc<ExpressionConstantNode>(ExpressionConstantNode::ConstantType::TRUE_T);
		}
		else if (rhIdent == "false") {
			constNode = alloc<ExpressionConstantNode>(ExpressionConstantNode::ConstantType::FALSE_T);
		}
		else if (rhIdent == "null") {
			constNode = alloc<ExpressionConstantNode>(ExpressionConstantNode::ConstantType::NULL_T);
		}
		else
		{
			// report error - constant does not exist
			printf("Constant %s does not exist, cant define it to %s\n", rhIdent.c_str(), ident.c_str());
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
		// report error - redefining constant
		return;
	}

	constantsTable[ident] = node;
}

ExpressionNode * ParserContext::getConstant(const std::string& key)
{
	auto it = constantsTable.find(key);
	if (it == constantsTable.end())
		return nullptr;

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
