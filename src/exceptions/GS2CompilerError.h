#pragma once

#ifndef GS2ERRORHANDLING_H
#define GS2ERRORHANDLING_H

#include <string>
#include "utils/EventHandler.h"

enum class ErrorLevel
{
	E_INFO,
	E_WARNING,
	E_ERROR,
	E_ALL
};

class GS2CompilerError
{
public:
	enum class ErrorCategory
	{
		Undefined,
		Parser,
		Compiler
	};

	GS2CompilerError(ErrorLevel level, ErrorCategory code, std::string msg)
			: _code(code), _level(level), _msg(std::move(msg))
	{

	}

	// disable copy constructor
	GS2CompilerError(const GS2CompilerError& o) = delete;
	GS2CompilerError& operator=(const GS2CompilerError& o) = delete;

	// enable move constructors
	GS2CompilerError(GS2CompilerError&& o) noexcept = default;
	GS2CompilerError& operator=(GS2CompilerError&& o) noexcept = default;

	const std::string& msg() const
	{
		return _msg;
	}

	ErrorCategory code() const
	{
		return _code;
	}

	ErrorLevel level() const
	{
		return _level;
	}

private:
	ErrorCategory _code;
	ErrorLevel _level;
	std::string _msg;
};

using GS2ErrorService = EventHandler<GS2CompilerError, ErrorLevel>;

#endif
