#pragma once

#ifndef GS2CONTEXT_H
#define GS2CONTEXT_H

#include <set>
#include <vector>
#include "encoding/buffer.h"
#include "exceptions/GS2CompilerError.h"
#include "GS2BuiltInFunctions.h"

struct CompilerResponse
{
	bool success;
	std::vector<GS2CompilerError> errors;

	Buffer bytecode;
	std::set<std::string> joinedClasses;
};

class GS2Context
{
	public:
		GS2Context();

		CompilerResponse compile(const std::string& script);
		CompilerResponse compile(const std::string& script, const std::string& scriptType, const std::string& scriptName, bool saveToDisk);

		static Buffer CreateHeader(const Buffer& bytecode, const std::string& scriptType, const std::string& scriptName, bool saveToDisk);
		static CompilerResponse Compile(const std::string &script, const std::string &scriptType, const std::string &scriptName, bool saveToDisk);

	private:
		GS2BuiltInFunctions builtIn;
		GS2ErrorService errorService;
		std::vector<GS2CompilerError> errors;

		/*
		 * Called whenever an error occurs during any stage of compilation,
		 * currently just appends the error to the errors vector to return
		 * in CompilerResponse
		 */
		void handleError(GS2CompilerError &error);
};

inline CompilerResponse GS2Context::compile(const std::string& script, const std::string& scriptType, const std::string& scriptName, bool saveToDisk)
{
	CompilerResponse results = compile(script);
	if (results.success)
		results.bytecode = CreateHeader(results.bytecode, scriptType, scriptName, saveToDisk);

	return results;
}

inline CompilerResponse GS2Context::Compile(const std::string &script, const std::string &scriptType, const std::string &scriptName, bool saveToDisk)
{
	GS2Context ctx;
	return ctx.compile(script, scriptType, scriptName, saveToDisk);
}

#endif
