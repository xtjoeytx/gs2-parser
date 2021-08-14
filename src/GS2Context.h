#pragma once

#ifndef GS2CONTEXT_H
#define GS2CONTEXT_H

#include "exceptions/GS2CompilerError.h"
#include "visitors/GS2CompilerVisitor.h"
#include "GS2BuiltInFunctions.h"

class GS2Context
{
	public:
		GS2Context()
		{
			builtIn = GS2BuiltInFunctions::getBuiltIn();
		}

		Buffer compile(const std::string& script);
		Buffer compile(const std::string& script, const std::string& scriptType, const std::string& scriptName, bool saveToDisk);

		static Buffer createHeader(const Buffer& bytecode, const std::string& scriptType, const std::string& scriptName, bool saveToDisk);

		bool hasErrors() const
		{
			return !errors.empty();
		}

		const std::vector<GS2CompilerError>& getErrors() const
		{
			return errors;
		}

	private:
		GS2BuiltInFunctions builtIn;
		std::vector<GS2CompilerError> errors;
};

inline Buffer GS2Context::compile(const std::string& script, const std::string& scriptType, const std::string& scriptName, bool saveToDisk)
{
	return createHeader(compile(script), scriptType, scriptName, saveToDisk);
}

#endif
