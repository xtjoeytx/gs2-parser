#include "GS2Context.h"
#include "encoding/graalencoding.h"
#include "visitors/GS2CompilerVisitor.h"
#include "Parser.h"

GS2Context::GS2Context()
	: errorService([this](auto && PH1) { handleError(std::forward<decltype(PH1)>(PH1)); })
{
	builtIn = GS2BuiltInFunctions::getBuiltIn();
}

void GS2Context::handleError(GS2CompilerError& error)
{
	errors.push_back(std::move(error));
}

CompilerResponse GS2Context::compile(const std::string& script)
{
	errors.clear();

	// Parse the script into an AST tree
	ParserContext parserContext(errorService);
	bool success = parserContext.parse(script);

	// Check for parser errors
	if (success)
	{
		// Grab the root node of the AST tree
		auto stmtBlock = parserContext.getRootStatement();

		if (stmtBlock)
		{
			// Walk the AST tree to produce bytecode
			GS2CompilerVisitor compilerVisitor(parserContext, builtIn);
			compilerVisitor.Visit(stmtBlock);

			return CompilerResponse{
				true,
				std::move(errors),
				compilerVisitor.getByteCode(),
				compilerVisitor.getJoinedClasses()
			};
		}
	}
	
	// If we have no errors, lets add one
	if (errors.empty())
		parserContext.addParserError("malformed input");
	
	return CompilerResponse{
		false,
		std::move(errors),
		Buffer{},
	};
}

Buffer GS2Context::CreateHeader(const Buffer& bytecode, const std::string& scriptType, const std::string& scriptName, bool saveToDisk)
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

	// Checksum or key for encrypted files
	// Needs to be new every time script gets generated, otherwise the client won't request updated script
	for (int i = 0; i < 10; i++)
		bytecodeWithHeader.Write<GraalByte>(rand() % 0xFF);

	// Write out the bytecode to the buffer
	bytecodeWithHeader.write(bytecode);
	return bytecodeWithHeader;
}
