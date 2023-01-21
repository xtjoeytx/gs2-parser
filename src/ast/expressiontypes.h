#pragma once 

#ifndef EXPRESSIONTYPES_H
#define EXPRESSIONTYPES_H


enum class ExpressionType
{
	EXPR_ANY,
	EXPR_INTEGER,
	EXPR_NUMBER,
	EXPR_STRING,
	EXPR_IDENT,
	EXPR_OBJECT,
	EXPR_ARRAY,
	EXPR_MULTIARRAY,
	EXPR_FUNCTION,
	EXPR_FUNCTIONOBJ
};

enum class ExpressionOp
{
	Plus,
	Minus,
	Multiply,
	Divide,
	Mod,
	Pow,
	Assign,
	Concat,
	Equal,
	NotEqual,
	LessThan,
	GreaterThan,
	LessThanOrEqual,
	GreaterThanOrEqual,
	LogicalAnd,
	LogicalOr,

	BitwiseAnd,
	BitwiseOr,
	BitwiseXor,
	BitwiseLeftShift,
	BitwiseRightShift,

	PlusAssign,
	MinusAssign,
	MultiplyAssign,
	DivideAssign,
	PowAssign,
	ModAssign,
	ConcatAssign,
	BitwiseLeftShiftAssign,
	BitwiseRightShiftAssign,

	UnaryStringCast,
	UnaryNot,
	UnaryMinus,
	BitwiseInvert,
	Increment,
	Decrement
};

inline bool IsNumberType(ExpressionType type)
{
	switch (type)
	{
		case ExpressionType::EXPR_INTEGER:
		case ExpressionType::EXPR_NUMBER:
			return true;

		default:
			return false;
	}
}

inline const char* ExpressionOpToString(ExpressionOp op)
{
	switch (op)
	{
		case ExpressionOp::Plus:
			return "+";

		case ExpressionOp::Minus:
			return "-";

		case ExpressionOp::Multiply:
			return "*";

		case ExpressionOp::Divide:
			return "/";

		case ExpressionOp::Mod:
			return "%";

		case ExpressionOp::Pow:
			return "^";

		case ExpressionOp::Assign:
			return "=";

		case ExpressionOp::Concat:
		case ExpressionOp::UnaryStringCast:
			return "@";

		case ExpressionOp::ConcatAssign:
			return "@=";

		case ExpressionOp::Equal:
			return "==";

		case ExpressionOp::NotEqual:
			return "!=";

		case ExpressionOp::LessThan:
			return "<";

		case ExpressionOp::GreaterThan:
			return ">";

		case ExpressionOp::LessThanOrEqual:
			return "<=";

		case ExpressionOp::GreaterThanOrEqual:
			return ">=";

		case ExpressionOp::LogicalAnd:
			return "&&";

		case ExpressionOp::LogicalOr:
			return "||";

		case ExpressionOp::UnaryNot:
			return "!";

		case ExpressionOp::UnaryMinus:
			return "-";

		case ExpressionOp::Increment:
			return "++";

		case ExpressionOp::Decrement:
			return "--";

		default:
			return "Unknown";
	}
}


#endif
