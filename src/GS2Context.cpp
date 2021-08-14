#include "GS2Context.h"

#include "encoding/graalencoding.h"
#include "GS2Bytecode.h"
#include "Parser.h"

Buffer GS2Context::compile(const std::string& script)
{
	// Parse the script into an AST tree
	ParserContext parserContext;
	parserContext.parse(script);

	// Check for parser errors
	if (!parserContext.hasErrors())
	{
		// Grab the root node of the AST tree
		auto stmtBlock = parserContext.getRootStatement();

		// Walk the AST tree to produce bytecode
		GS2CompilerVisitor compilerVisitor(parserContext, builtIn);
		compilerVisitor.Visit(stmtBlock);

		// Check for compiler errors
		if (!parserContext.hasErrors())
			return compilerVisitor.getByteCode();
	}

	// Move the parsers errors into the context
	errors = std::move(parserContext.getErrorList());

	if (errors.empty())
		errors.push_back(GS2CompilerError(GS2CompilerError::ErrorCode::ParserError, "Failed to parse script"));

	// Empty buffer indicates failure
	return {};
}

Buffer GS2Context::createHeader(const Buffer& bytecode, const std::string& scriptType, const std::string& scriptName, bool saveToDisk)
{
	// Empty bytecode buffer indicates there was a compilation error
	if (!bytecode.length())
		return {};

	// Precalculating the lengths to reduce allocations
	auto startSectionLength = scriptType.length() + scriptName.length() + 4 + 10;
	auto headerLength = 2 + startSectionLength + bytecode.length();

	Buffer bytecodeWithHeader(headerLength);

	// Write the length of the header section
	bytecodeWithHeader.Write<GraalShort>(uint16_t(startSectionLength));

	// Create the sections header
	bytecodeWithHeader.write(scriptType.c_str(), scriptType.length());
	bytecodeWithHeader.write(',');
	bytecodeWithHeader.write(scriptName.c_str(), scriptName.length());
	bytecodeWithHeader.write(',');
	bytecodeWithHeader.write(saveToDisk ? '1' : '0');
	bytecodeWithHeader.write(',');

	// Checksum or key for encrypted files, writing 0's has not
	// been an issue yet
	for (int i = 0; i < 10; i++)
		bytecodeWithHeader.Write<GraalByte>(0);
	
	// Write out the bytecode to the buffer
	bytecodeWithHeader.write(bytecode);
	return bytecodeWithHeader;
}
