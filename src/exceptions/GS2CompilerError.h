#pragma once

#ifndef GS2ERRORHANDLING_H
#define GS2ERRORHANDLING_H

#include <string>

class GS2CompilerError
{
public:
	enum class ErrorCode
	{
		UnknownError,
		ParserError,
		CompileError
	};

	GS2CompilerError(ErrorCode code, std::string msg)
		: _code(code), _msg(std::move(msg))
	{

	}

	// disable copy constructor
	GS2CompilerError(const GS2CompilerError& o) = delete;
	GS2CompilerError& operator=(const GS2CompilerError& o) = delete;

	// enable move constructors
	GS2CompilerError(GS2CompilerError&& o) noexcept = default;
	GS2CompilerError& operator=(GS2CompilerError&& o) noexcept = default;

	const std::string& msg() const {
		return _msg;
	}

	ErrorCode code() const {
		return _code;
	}

private:
	ErrorCode _code;
	std::string _msg;
};

#endif
