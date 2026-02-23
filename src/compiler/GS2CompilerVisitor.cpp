#include <cassert>
#include <algorithm>

#include "ast/ast.h"
#include "visitors/FunctionInspectVisitor.h"
#include "compiler/GS2CompilerVisitor.h"
#include "compiler/GS2BuiltInFunctions.h"
#include "parser/Parser.h"

opcode::Opcode getExpressionOpCode(ExpressionOp op)
{
	switch (op)
	{
		case ExpressionOp::Plus: return opcode::Opcode::OP_ADD;
		case ExpressionOp::Minus: return opcode::Opcode::OP_SUB;
		case ExpressionOp::Multiply: return opcode::Opcode::OP_MUL;
		case ExpressionOp::Divide: return opcode::Opcode::OP_DIV;
		case ExpressionOp::Mod: return opcode::Opcode::OP_MOD;
		case ExpressionOp::Pow: return opcode::Opcode::OP_POW;
		case ExpressionOp::BitwiseAnd: return opcode::Opcode::OP_BWA;
		case ExpressionOp::BitwiseOr: return opcode::Opcode::OP_BWO;
		case ExpressionOp::BitwiseXor: return opcode::Opcode::OP_BWX;
		case ExpressionOp::BitwiseLeftShift: return opcode::Opcode::OP_BW_LEFTSHIFT;
		case ExpressionOp::BitwiseRightShift: return opcode::Opcode::OP_BW_RIGHTSHIFT;
		case ExpressionOp::BitwiseInvert: return opcode::Opcode::OP_BWI;
		case ExpressionOp::Assign: return opcode::Opcode::OP_ASSIGN;
		case ExpressionOp::Equal: return opcode::Opcode::OP_EQ;
		case ExpressionOp::NotEqual: return opcode::Opcode::OP_NEQ;
		case ExpressionOp::LessThan: return opcode::Opcode::OP_LT;
		case ExpressionOp::LessThanOrEqual: return opcode::Opcode::OP_LTE;
		case ExpressionOp::GreaterThan: return opcode::Opcode::OP_GT;
		case ExpressionOp::GreaterThanOrEqual: return opcode::Opcode::OP_GTE;

		case ExpressionOp::PlusAssign: return opcode::Opcode::OP_ADD;
		case ExpressionOp::MinusAssign: return opcode::Opcode::OP_SUB;
		case ExpressionOp::MultiplyAssign: return opcode::Opcode::OP_MUL;
		case ExpressionOp::ModAssign: return opcode::Opcode::OP_MOD;
		case ExpressionOp::DivideAssign: return opcode::Opcode::OP_DIV;
		case ExpressionOp::ConcatAssign: return opcode::Opcode::OP_JOIN;
		case ExpressionOp::BitwiseLeftShiftAssign: return opcode::Opcode::OP_BW_LEFTSHIFT;
		case ExpressionOp::BitwiseRightShiftAssign: return opcode::Opcode::OP_BW_RIGHTSHIFT;

		case ExpressionOp::UnaryMinus: return opcode::Opcode::OP_UNARYSUB;
		case ExpressionOp::UnaryNot: return opcode::Opcode::OP_NOT;
		case ExpressionOp::Increment: return opcode::Opcode::OP_INC;
		case ExpressionOp::Decrement: return opcode::Opcode::OP_DEC;

		default: return opcode::Opcode::OP_NONE;
	}
}

GS2CompilerVisitor::GS2CompilerVisitor(ParserContext & context)
	: parserContext(context)
{
}

void GS2CompilerVisitor::Visit(Node *node)
{
	std::string errorMsg = std::format("unimplemented node type {}", node->NodeType());
	parserContext.addError({ ErrorLevel::E_ERROR, GS2CompilerError::ErrorCategory::Compiler, errorMsg });

#ifdef DBGEMITTERS
	fprintf(stderr, "%s\n", errorMsg.c_str());

	#ifdef _WIN32
		system("pause");
	#endif
	exit(1);
#endif
}

void GS2CompilerVisitor::Visit(StatementBlock *node)
{
	if (!_isRootBlock)
	{
		for (const auto& n : node->statements)
		{
			assert(n != nullptr);
			n->visit(this);
		}
		return;
	}

	_isRootBlock = false;
	JumpTarget fnSkip(byteCode);
	fn_skip_target = &fnSkip;

	for (const auto& n : node->statements)
	{
		assert(n != nullptr);

		size_t pendingBefore = fnSkip.pendingCount();
		auto opIndexBefore = byteCode.getOpIndex();

		n->visit(this);

		bool addedPrejump = (fnSkip.pendingCount() > pendingBefore);

		if (!addedPrejump && fnSkip.pendingCount() > 0)
		{
			fnSkip.resolve(opIndexBefore);
			fnSkip.reset();
		}
	}

	// Trailing OP_RET fixes a weird bug in which the last function was
	// uncallable on certain clients. Emitted here (before resolving the
	// final function-skip group) so that the pre-jump lands after it,
	// matching the legacy target address. - joey
	byteCode.emit(opcode::OP_RET);

	if (fnSkip.pendingCount() > 0)
		fnSkip.resolveHere();

	fn_skip_target = nullptr;
}

void GS2CompilerVisitor::Visit(StatementFnDeclNode *node)
{
#ifdef DBGEMITTERS
	printf("Declare function: %s\n", node->ident->c_str());
#endif

	if (node->emit_prejump && fn_skip_target)
	{
		fn_skip_target->emitJump(opcode::OP_SET_INDEX);
	}

	bool is_universe = false;
	std::string funcName;
	if (node->pub)
		funcName.append("public.");

	if (node->objectName && !node->objectName->empty())
	{
		if (node->objectName->length() == 8 && *node->objectName == "universe")
			is_universe = true;

		funcName.append(*node->objectName).append(".");
	}
	funcName.append(*node->ident);

	if (is_universe)
	{
		std::string funcNameUni;
		if (node->pub)
			funcNameUni.append("public.");

		funcNameUni.append(*node->ident).append(",").append(funcName);
		std::swap(funcName, funcNameUni);
	}

	byteCode.addFunction(funcName, byteCode.getOpIndex());

	{
		byteCode.emit(opcode::OP_TYPE_ARRAY);

		for (auto it = node->args.rbegin(); it != node->args.rend(); ++it)
		{
			assert(*it != nullptr);

			(*it)->visit(this);
		}

		byteCode.emit(opcode::OP_FUNC_PARAMS_END);
	}

	byteCode.emit(opcode::OP_JMP);

	// Found plenty of examples of OP_CMD_CALL being excluded, none of the functions
	// that excluded the opcode had function calls so we are replicating that behavior
	{
		FunctionInspectVisitor funcVisitor;
		funcVisitor.Visit(node->stmtBlock);

		if (funcVisitor.foundFunctionCall)
			byteCode.emit(opcode::OP_CMD_CALL);
	}

	node->stmtBlock->visit(this);

	// if our last op was a return statement, we can skip writing a duplicate
	// return statement at the end of the function
	if (byteCode.getLastOp() != opcode::OP_RET)
	{
		byteCode.emit(opcode::OP_TYPE_NUMBER);
		byteCode.emitDynamicNumber(0);
		byteCode.emit(opcode::OP_RET);
	}
}

void GS2CompilerVisitor::Visit(ExpressionTernaryOpNode *node)
{
	node->condition->visit(this);

	ScopeGuard guard(*this);

	JumpTarget falseBranch(byteCode);
	JumpTarget end(byteCode);

	{
		fail_target = &falseBranch;
		success_target = &end;

		// Convert the result of the expression to a number since this
		// value will be used for the following if () stmt
		if (!IsBooleanReturningOp(byteCode.getLastOp()))
			byteCode.emitConversionOp(node->condition->expressionType(), ExpressionType::EXPR_NUMBER);

		falseBranch.emitJump(opcode::OP_IF);

		node->leftExpr->visit(this);

		// set the continue position to the right-hand expression, skipping
		// over the jump on the left-hand expression
		falseBranch.resolve(byteCode.getOpIndex() + 1);
	}

	// emit a jump to the end of this else block for the previous if-block
	end.emitJump(opcode::OP_SET_INDEX);

	node->rightExpr->visit(this);
	end.resolveHere();
}

void GS2CompilerVisitor::Visit(ExpressionBinaryOpNode *node)
{
	if (node->op == ExpressionOp::LogicalAnd || node->op == ExpressionOp::LogicalOr)
	{
		ScopeGuard guard(*this);

		JumpTarget inlineTarget(byteCode);
		bool isFirstBinaryExpr = !_isInsideExpression;
		if (isFirstBinaryExpr)
		{
			_isInsideExpression = true;

			// If its a conditional expression we need to create new jump labels which leads
			// to OP_INLINE_CONDITIONAL emitted at the end of the first binary expression
			if (_isInlineConditional)
			{
				success_target = &inlineTarget;
				fail_target = &inlineTarget;
			}
		}

		auto* tmp_success = success_target;
		auto* tmp_fail = fail_target;
		bool is_inline_cond = _isInlineConditional;

		if (node->op == ExpressionOp::LogicalAnd)
		{
			JumpTarget afterLhs(byteCode);
			success_target = &afterLhs;

			node->left->visit(this);
			byteCode.emitConversionOp(node->left->expressionType(), ExpressionType::EXPR_NUMBER);

			afterLhs.resolveHere();
			success_target = tmp_success;
			fail_target = tmp_fail;

			if (is_inline_cond)
				fail_target->emitJump(opcode::OP_AND);
			else
				fail_target->emitJump(opcode::OP_IF);

			node->right->visit(this);
			byteCode.emitConversionOp(node->right->expressionType(), ExpressionType::EXPR_NUMBER);
		}
		else if (node->op == ExpressionOp::LogicalOr)
		{
			JumpTarget afterLhs(byteCode);
			fail_target = &afterLhs;

			node->left->visit(this);
			byteCode.emitConversionOp(node->left->expressionType(), ExpressionType::EXPR_NUMBER);

			success_target->emitJump(opcode::OP_OR);

			afterLhs.resolveHere();
			success_target = tmp_success;
			fail_target = tmp_fail;

			node->right->visit(this);
			byteCode.emitConversionOp(node->right->expressionType(), ExpressionType::EXPR_NUMBER);
		}

		if (isFirstBinaryExpr)
		{
			_isInsideExpression = false;

			if (is_inline_cond)
			{
				inlineTarget.resolveHere();
				byteCode.emit(opcode::Opcode::OP_INLINE_CONDITIONAL);
			}
		}

		return;
	}

	switch (node->op)
	{
		case ExpressionOp::Plus:
		case ExpressionOp::Minus:
		case ExpressionOp::Multiply:
		case ExpressionOp::Divide:
		case ExpressionOp::Mod:
		case ExpressionOp::Pow:
		case ExpressionOp::BitwiseAnd:
		case ExpressionOp::BitwiseOr:
		case ExpressionOp::BitwiseXor:
		case ExpressionOp::BitwiseLeftShift:
		case ExpressionOp::BitwiseRightShift:
		case ExpressionOp::LessThan:
		case ExpressionOp::LessThanOrEqual:
		case ExpressionOp::GreaterThan:
		case ExpressionOp::GreaterThanOrEqual:
		{
			node->left->visit(this);
			byteCode.emitConversionOp(node->left->expressionType(), ExpressionType::EXPR_NUMBER);
			node->right->visit(this);
			byteCode.emitConversionOp(node->right->expressionType(), ExpressionType::EXPR_NUMBER);

			auto opCode = getExpressionOpCode(node->op);
			assert(opCode != opcode::Opcode::OP_NONE);

			byteCode.emit(opCode);
			return;
		}

		case ExpressionOp::Equal:
		case ExpressionOp::NotEqual:
		{
			node->left->visit(this);
			node->right->visit(this);

			auto opCode = getExpressionOpCode(node->op);
			assert(opCode != opcode::Opcode::OP_NONE);

			byteCode.emit(opCode);
			return;
		}

		case ExpressionOp::PlusAssign:
		case ExpressionOp::MinusAssign:
		case ExpressionOp::MultiplyAssign:
		case ExpressionOp::DivideAssign:
		case ExpressionOp::ModAssign:
		case ExpressionOp::BitwiseLeftShiftAssign:
		case ExpressionOp::BitwiseRightShiftAssign:
		{
			// Visit left operand, and copy it. Cast to number for operation
			node->left->visit(this);
			byteCode.emit(opcode::Opcode::OP_COPY_LAST_OP);
			byteCode.emitConversionOp(node->left->expressionType(), ExpressionType::EXPR_NUMBER);

			// Visit right operand
			node->right->visit(this);
			byteCode.emitConversionOp(node->right->expressionType(), ExpressionType::EXPR_NUMBER);

			// Emit the operation sign ('+', '-', '*', '/')
			auto opCode = getExpressionOpCode(node->op);
			assert(opCode != opcode::Opcode::OP_NONE);
			byteCode.emit(opCode);

			// Special assignment operators for array/multi-dimensional arrays
			auto exprType = node->left->expressionType();
			if (exprType == ExpressionType::EXPR_ARRAY)
				opCode = opcode::Opcode::OP_ARRAY_ASSIGN;
			else if (exprType == ExpressionType::EXPR_MULTIARRAY)
				opCode = opcode::Opcode::OP_ARRAY_MULTIDIM_ASSIGN;
			else
				opCode = opcode::OP_ASSIGN;

			byteCode.emit(opCode);
			return;
		}

		case ExpressionOp::ConcatAssign:
		{
			node->left->visit(this);
			byteCode.emit(opcode::Opcode::OP_COPY_LAST_OP);
			byteCode.emitConversionOp(node->left->expressionType(), ExpressionType::EXPR_STRING);

			auto opCode = getExpressionOpCode(node->op);
			assert(opCode == opcode::Opcode::OP_JOIN);

			node->right->visit(this);
			byteCode.emitConversionOp(node->right->expressionType(), ExpressionType::EXPR_STRING);
			byteCode.emit(opCode);

			// Special assignment operators for array/multi-dimensional arrays
			auto exprType = node->left->expressionType();
			if (exprType == ExpressionType::EXPR_ARRAY)
				opCode = opcode::Opcode::OP_ARRAY_ASSIGN;
			else if (exprType == ExpressionType::EXPR_MULTIARRAY)
				opCode = opcode::Opcode::OP_ARRAY_MULTIDIM_ASSIGN;
			else
				opCode = opcode::OP_ASSIGN;

			byteCode.emit(opCode);
			return;
		}

		case ExpressionOp::Assign:
		{
			node->left->visit(this);

			// if the parent, and the next node are both assignments we need to
			// copy the value on the top of the stack before the next assignment op
			{
				if (_isCopyAssignment)
				{
					byteCode.emit(opcode::OP_COPY_LAST_OP);
					_isCopyAssignment = false;
				}

				if (node->right->isAssignment)
					_isCopyAssignment = true;
			}

			opcode::Opcode opCode;
			if (node->op != ExpressionOp::Assign)
			{
				opCode = opcode::Opcode::OP_ASSIGN;
				byteCode.emit(getExpressionOpCode(node->op));
			}
			else
			{
				opCode = getExpressionOpCode(node->op);
				assert(opCode != opcode::Opcode::OP_NONE);
			}

			node->right->visit(this);

			// Special assignment operators for array/multi-dimensional arrays
			auto exprType = node->left->expressionType();
			if (exprType == ExpressionType::EXPR_ARRAY)
				opCode = opcode::Opcode::OP_ARRAY_ASSIGN;
			else if (exprType == ExpressionType::EXPR_MULTIARRAY)
				opCode = opcode::Opcode::OP_ARRAY_MULTIDIM_ASSIGN;

			byteCode.emit(opCode);
			return;
		}
	}

	std::string errorMsg = std::format("Undefined opcode in BinaryExpression {}: {} {}", static_cast<int>(node->op), std::string{ExpressionOpToString(node->op)}, node->toString());
	parserContext.addError({ ErrorLevel::E_ERROR, GS2CompilerError::ErrorCategory::Compiler, std::move(errorMsg) });
}

void GS2CompilerVisitor::Visit(ExpressionUnaryOpNode* node)
{
	// If the expression is a constant, we can apply the negative now to the expression
	if (node->op == ExpressionOp::UnaryMinus)
	{
		auto nodeType = node->expr->NodeType();

		if (nodeType == ExpressionIntegerNode::NodeName)
		{
			auto underlying_node = reinterpret_cast<ExpressionIntegerNode *>(node->expr);
			underlying_node->val = -underlying_node->val;
			underlying_node->visit(this);
			return;
		}
		else if (nodeType == ExpressionNumberNode::NodeName)
		{
			auto underlying_node = reinterpret_cast<ExpressionNumberNode *>(node->expr);
			underlying_node->val->insert(0, "-");
			underlying_node->visit(this);
			return;
		}
	}

	auto temp_inlinecond = _isInlineConditional;
	ScopeGuard guard(*this);

	JumpTarget inlineTarget(byteCode);
	bool isFirstBinaryExpr = !_isInsideExpression;
	if (isFirstBinaryExpr)
	{
		_isInsideExpression = true;
		_isInlineConditional = true;

		success_target = &inlineTarget;
		fail_target = &inlineTarget;
	}

	node->expr->visit(this);

	if (isFirstBinaryExpr)
	{
		_isInsideExpression = false;
		_isInlineConditional = temp_inlinecond;

		inlineTarget.resolveHere();
	}

	if (node->opFirst)
	{
		switch (node->op)
		{
			case ExpressionOp::Increment:
			case ExpressionOp::Decrement:
			{
				auto opCode = getExpressionOpCode(node->op);
				assert(opCode != opcode::Opcode::OP_NONE);

				byteCode.emit(opCode);
				if (node->opUnused)
					byteCode.emit(opcode::OP_INDEX_DEC);

				return;
			}

			case ExpressionOp::UnaryMinus:
			case ExpressionOp::UnaryNot:
			case ExpressionOp::BitwiseInvert:
			{
				auto opCode = getExpressionOpCode(node->op);
				assert(opCode != opcode::Opcode::OP_NONE);

				if (!IsBooleanReturningOp(byteCode.getLastOp()))
					byteCode.emitConversionOp(node->expr->expressionType(), ExpressionType::EXPR_NUMBER);

				byteCode.emit(opCode);
				return;
			}

			case ExpressionOp::Plus:
			{
				auto opCode = getExpressionOpCode(node->op);
				assert(opCode != opcode::Opcode::OP_NONE);

				if (!IsBooleanReturningOp(byteCode.getLastOp()))
					byteCode.emitConversionOp(node->expr->expressionType(), ExpressionType::EXPR_NUMBER);

				byteCode.emit(opCode);
				return;
			}

			case ExpressionOp::UnaryStringCast:
			{
				byteCode.emit(opcode::OP_CONV_TO_STRING);

				// need to test to see if this should always be emitted here, or in postfixnode
				if (node->expr->expressionType() == ExpressionType::EXPR_ARRAY)
					byteCode.emit(opcode::OP_MEMBER_ACCESS);

				return;
			}

		default:
			return;
		}
	}

	switch (node->op)
	{
		case ExpressionOp::Increment:
		case ExpressionOp::Decrement:
		{
			auto opCode = getExpressionOpCode(node->op);
			assert(opCode != opcode::Opcode::OP_NONE);

			// TODO(joey): need to fix
			byteCode.emit(opcode::OP_COPY_LAST_OP);
			byteCode.emit(opcode::OP_CONV_TO_FLOAT);
			byteCode.emit(opcode::OP_SWAP_LAST_OPS);
			byteCode.emit(opCode);
			byteCode.emit(opcode::OP_INDEX_DEC);
			return;
		}
		default:
		{
			std::string errorMsg = std::format("Undefined opcode in UnaryExpression {}: {}", static_cast<int>(node->op), ExpressionOpToString(node->op));
			parserContext.addError({ ErrorLevel::E_ERROR, GS2CompilerError::ErrorCategory::Compiler, std::move(errorMsg) });
			return;
		}
	}
}

void GS2CompilerVisitor::Visit(ExpressionStrConcatNode *node)
{
	node->left->visit(this);
	byteCode.emitConversionOp(node->left->expressionType(), ExpressionType::EXPR_STRING);

	switch (node->sep)
	{
		case ' ':
		case '\t':
		case '\n':
			auto id = byteCode.getStringConst(std::string(1, node->sep));
			byteCode.emit(opcode::OP_TYPE_STRING);
			byteCode.emitDynamicNumberUnsigned(id);

			byteCode.emit(opcode::OP_JOIN);
			break;
	}

	node->right->visit(this);
	byteCode.emitConversionOp(node->right->expressionType(), ExpressionType::EXPR_STRING);

	byteCode.emit(opcode::OP_JOIN);
}

void GS2CompilerVisitor::Visit(ExpressionCastNode* node)
{
	node->expr->visit(this);

	switch (node->type)
	{
		case ExpressionCastNode::CastType::INTEGER:
			byteCode.emitConversionOp(node->expr->expressionType(), ExpressionType::EXPR_NUMBER);
			byteCode.emit(opcode::OP_INT);
			break;

		case ExpressionCastNode::CastType::FLOAT:
			byteCode.emit(opcode::OP_CONV_TO_FLOAT);
			break;

		case ExpressionCastNode::CastType::STRING:
			byteCode.emit(opcode::OP_CONV_TO_STRING);
			break;

		case ExpressionCastNode::CastType::TRANSLATION:
			byteCode.emitConversionOp(node->expr->expressionType(), ExpressionType::EXPR_STRING);
			byteCode.emit(opcode::OP_TRANSLATE);
			break;
	}
}

void GS2CompilerVisitor::Visit(ExpressionArrayIndexNode *node)
{
	for (const auto& expr : node->exprList)
	{
		expr->visit(this);
		byteCode.emitConversionOp(expr->expressionType(), ExpressionType::EXPR_NUMBER);
	}

	if (!node->isAssignment)
	{
		if (node->expressionType() == ExpressionType::EXPR_MULTIARRAY)
			byteCode.emit(opcode::OP_ARRAY_MULTIDIM);
		else
			byteCode.emit(opcode::OP_ARRAY);
	}
}

void GS2CompilerVisitor::Visit(ExpressionInOpNode *node)
{
	// expr in |lower, higher|
	// expr in obj - obj = lower

	node->expr->visit(this);
	node->lower->visit(this);

	if (node->higher)
	{
		byteCode.emitConversionOp(node->lower->expressionType(), ExpressionType::EXPR_NUMBER);
		node->higher->visit(this);
		byteCode.emitConversionOp(node->higher->expressionType(), ExpressionType::EXPR_NUMBER);

		byteCode.emit(opcode::OP_IN_RANGE);
	}
	else
	{
		byteCode.emitConversionOp(node->lower->expressionType(), ExpressionType::EXPR_OBJECT);
		byteCode.emit(opcode::OP_IN_OBJ);
	}
}

void GS2CompilerVisitor::Visit(ExpressionConstantNode *node)
{
	switch (node->type)
	{
		case ExpressionConstantNode::ConstantType::TRUE_T:
			byteCode.emit(opcode::OP_TYPE_TRUE);
			break;

		case ExpressionConstantNode::ConstantType::FALSE_T:
			byteCode.emit(opcode::OP_TYPE_FALSE);
			break;

		case ExpressionConstantNode::ConstantType::NULL_T:
			byteCode.emit(opcode::OP_TYPE_NULL);
			break;
	}
}

void GS2CompilerVisitor::Visit(ExpressionIdentifierNode *node)
{
	static std::unordered_map<std::string, opcode::Opcode> identMappings = {
		{"this", opcode::OP_THIS},
		{"thiso", opcode::OP_THISO},
		{"player", opcode::OP_PLAYER},
		{"playero", opcode::OP_PLAYERO},
		{"level", opcode::OP_LEVEL},
		{"temp", opcode::OP_TEMP},
		{"true", opcode::OP_TYPE_TRUE},
		{"false", opcode::OP_TYPE_FALSE},
		{"null", opcode::OP_TYPE_NULL},
		{"pi", opcode::OP_PI}
	};

	// This is only true for the leading identifier
	// this.testobj.field, it would be true for the first node (this) but false for
	// the second node (testobj) and third node (field) that way reserved keywords
	// can technically be used in field names. don't recommend, but it should work
	if (node->checkForReservedIdents)
	{
		auto identIter = identMappings.find(*node->val);
		if (identIter != identMappings.end())
		{
			byteCode.emit(identIter->second);
			return;
		}
	}

	// TODO(joey): This may need to be included in the above check
	auto constant = parserContext.getConstant(*node->val);
	if (constant)
	{
		constant->visit(this);
		return;
	}

	auto id = byteCode.getStringConst(*node->val);

	byteCode.emit(opcode::OP_TYPE_VAR);
	byteCode.emitDynamicNumberUnsigned(id);
}

void GS2CompilerVisitor::Visit(ExpressionIntegerNode *node)
{
	byteCode.emit(opcode::OP_TYPE_NUMBER);
	byteCode.emitDynamicNumber(node->val);
}

void GS2CompilerVisitor::Visit(ExpressionNumberNode *node)
{
	byteCode.emit(opcode::OP_TYPE_NUMBER);
	byteCode.emitDoubleNumber(*node->val);
}

void GS2CompilerVisitor::Visit(ExpressionPostfixNode* node)
{
	assert(!node->nodes.empty());

	// mark our last node as an assignment
	if (node->isAssignment)
		node->nodes.back()->isAssignment = true;

	auto count = node->nodes.size();
	for (auto i = 0; i < count; i++)
	{
		node->nodes[i]->visit(this);

		auto exprType = node->nodes[i]->expressionType();
		if (!(exprType == ExpressionType::EXPR_ARRAY || exprType == ExpressionType::EXPR_MULTIARRAY))
		{
			if (i == 0)
			{
				if (count > 1 && !IsObjectReturningOp(byteCode.getLastOp()))
					/* && byteCode.getLastOp() == opcode::OP_TYPE_VAR*/
				{
					byteCode.emit(opcode::OP_CONV_TO_OBJECT);
				}
			}
			else if (i > 0)
			{
				byteCode.emit(opcode::OP_MEMBER_ACCESS);
				if (i < count - 1)
				{
					byteCode.emit(opcode::OP_CONV_TO_OBJECT);
				}
			}
		}
	}
}

void GS2CompilerVisitor::Visit(ExpressionStringConstNode *node)
{
#ifdef DBGEMITTERS
	printf("String: %s\n", node->val->c_str());
#endif

	auto id = byteCode.getStringConst(*node->val);

	byteCode.emit(opcode::OP_TYPE_STRING);
	byteCode.emitDynamicNumberUnsigned(id);
}

ExpressionType getSigType(char idx) {
	// x -> NONE
	// f -> OP_CONV_TO_FLOAT
	// o -> OP_CONV_TO_OBJECT
	// s -> OP_CONV_TO_STRING

	switch (idx) {
		case 'f': return ExpressionType::EXPR_NUMBER;
		case 'o': return ExpressionType::EXPR_OBJECT;
		case 's': return ExpressionType::EXPR_STRING;
		default: return ExpressionType::EXPR_ANY;
	}
}

void GS2CompilerVisitor::Visit(ExpressionFnCallNode *node)
{
	auto isObjectCall = (node->objExpr != nullptr);

	// Built-in commands
	std::string funcName = node->funcExpr->toString();

#ifdef DBGEMITTERS
	printf("Call Function: %s (obj call: %d)\n", funcName.c_str(), isObjectCall ? 1 : 0);
#endif

	const BuiltInCmd* found = isObjectCall ? findBuiltInObjCmd(funcName) : findBuiltInCmd(funcName);
	BuiltInCmd cmd = found ? *found : (isObjectCall ? defaultObjCall : defaultCall);

	{
		auto argumentVisitFn = [&](auto arg_iter, auto arg_iter_end, auto sig_iter, auto sig_iter_end) {
			const auto& sig = cmd.sig;

			// we need to skip over the return value
			if (sig_iter != sig_iter_end)
				++sig_iter;

			for (; arg_iter != arg_iter_end; ++arg_iter)
			{
				char sig_ch = 'x';
				if (sig_iter != sig_iter_end)
				{
					sig_ch = *sig_iter;
					++sig_iter;
				}

				ExpressionNode* node = *arg_iter;
				node->visit(this);
				byteCode.emitConversionOp(node->expressionType(), getSigType(sig_ch));
			}
		};

		auto objectVisitFn = [&]() {
			if (isObjectCall)
				node->objExpr->visit(this);

			// Convert the object to a specific type
			// Explanation: Some functions (like string functions, ex: str.substr(start, end)) are really passed as
			// substr(str, start, end) and so we need to ensure the first argument is in fact a string so we convert that here.
			if (cmd.convert_object_op != opcode::Opcode::OP_NONE && byteCode.getLastOp() != cmd.convert_object_op)
			{
				if (cmd.convert_object_op == opcode::Opcode::OP_CONV_TO_OBJECT)
				{
					if (!IsObjectReturningOp(byteCode.getLastOp()))
					{
						byteCode.emit(cmd.convert_object_op);
					}
				}
				else byteCode.emit(cmd.convert_object_op);
			}
		};

		if ((cmd.flags & CmdFlags::CMD_OBJECT_FIRST) == CmdFlags::CMD_OBJECT_FIRST)
		{
			objectVisitFn();
		}

		if ((cmd.flags & CmdFlags::CMD_USE_ARRAY) == CmdFlags::CMD_USE_ARRAY)
		{
			byteCode.emit(opcode::OP_TYPE_ARRAY);
		}

		if ((cmd.flags & CmdFlags::CMD_REVERSE_ARGS) == CmdFlags::CMD_REVERSE_ARGS)
		{
			// note: reversing both args and signature
			argumentVisitFn(node->args.rbegin(), node->args.rend(), cmd.sig.rbegin(), cmd.sig.rend());
		}
		else
		{
			// obj.tokenize has a default parameter of " ,"
			if (node->args.empty() && cmd.op == opcode::Opcode::OP_OBJ_TOKENIZE)
			{
				auto id = byteCode.getStringConst(" ,");
				byteCode.emit(opcode::OP_TYPE_STRING);
				byteCode.emitDynamicNumberUnsigned(id);
			}
			else
			{
				argumentVisitFn(node->args.begin(), node->args.end(), cmd.sig.begin(), cmd.sig.end());
			}
		}

		if ((cmd.flags & CmdFlags::CMD_OBJECT_FIRST) != CmdFlags::CMD_OBJECT_FIRST)
		{
			objectVisitFn();
		}

		if (cmd.op == opcode::OP_CALL)
		{
			node->funcExpr->visit(this);

			if (isObjectCall)
				byteCode.emit(opcode::OP_MEMBER_ACCESS);
		}

		byteCode.emit(cmd.op);
	}

	// Handling of discarding unused return values
	//
	// We need to pop the return value off the stack if the value is not going
	// to be used in the next op. All function calls are wrapped in an ExpressionPostfixNode,
	// and if the parent node of the postfix node is a statement block then we can assume
	// the return value is unused and emit OP_INDEX_DEC to pop the value off the stack.
	if ((cmd.flags & CmdFlags::CMD_RETURN_VALUE) == CmdFlags::CMD_RETURN_VALUE)
	{
		Node *checkNode = node->parent;
		assert(checkNode);

		if (checkNode->NodeType() == ExpressionPostfixNode::NodeName)
			checkNode = checkNode->parent;

		if (checkNode->NodeType() == StatementBlock::NodeName)
		{
			byteCode.emit(opcode::OP_INDEX_DEC);
		}
	}

	// Any scripts that are joined by this npc/weapon need to be compiled and sent before
	// the client can execute this script, so we are keeping track of any calls to join
	// with string constants so we can precompile these scripts and send them ahead of time
	if (funcName == "join")
	{
		if (node->args.size() == 1)
		{
			if (node->args[0]->expressionType() == ExpressionType::EXPR_STRING)
			{
				joinedClasses.insert(node->args[0]->toString());
			}
		}
	}
}

void GS2CompilerVisitor::Visit(ExpressionFnObject *node)
{
	// We are emitting the jump before the function-decl node
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));
	auto jmpLoc = byteCode.getBytecodePos();

	// Visit the function declaration node
	Visit(&node->fnNode);

	// Emit jump for the above index, skipping over the lambda function
	byteCode.emit(short(byteCode.getOpIndex()), jmpLoc - 2);

	// this
	byteCode.emit(opcode::OP_THIS);

	// assigned anonymous function name
	auto id = byteCode.getStringConst(*node->ident);
	byteCode.emit(opcode::OP_TYPE_VAR);
	byteCode.emitDynamicNumberUnsigned(id);

	// access this.function_name as an object
	byteCode.emit(opcode::OP_MEMBER_ACCESS);
	byteCode.emit(opcode::OP_CONV_TO_OBJECT);
}

void GS2CompilerVisitor::Visit(StatementReturnNode *node)
{
	if (node->expr)
	{
		ScopeGuard guard(*this);
		JumpTarget exprEnd(byteCode);
		success_target = &exprEnd;
		fail_target = &exprEnd;

		node->expr->visit(this);

		exprEnd.resolveHere();
	}
	else
	{
		byteCode.emit(opcode::OP_TYPE_NUMBER);
		byteCode.emitDynamicNumber(0);
	}

	byteCode.emit(opcode::OP_RET);
}

void GS2CompilerVisitor::Visit(StatementIfNode* node)
{
	ScopeGuard guard(*this);

	JumpTarget condEnd(byteCode);
	JumpTarget falseBranch(byteCode);

	{
		success_target = &condEnd;
		fail_target = &falseBranch;

		{
			_isInlineConditional = false;
			node->expr->visit(this);
			_isInlineConditional = true;
		}

		// Convert the result of the expression to a number since this
		// value will be used for the following if () stmt
		if (!IsBooleanReturningOp(byteCode.getLastOp()))
			byteCode.emitConversionOp(node->expr->expressionType(), ExpressionType::EXPR_NUMBER);

		// set the break point to the start of the OP_IF instruction
		condEnd.resolveHere();

		falseBranch.emitJump(opcode::OP_IF);

		node->thenBlock->visit(this);

		// OP_IF jumps to this location if the condition is false, so we
		// continue to the next instruction, but if their is an else-block we must
		// skip the next instruction since its a jmp to the end of the if-else chain
		auto nextOpcode = byteCode.getOpIndex() + (node->elseBlock ? 1 : 0);
		falseBranch.resolve(nextOpcode);
	}

	if (node->elseBlock)
	{
		// emit a jump to the end of this else block for the previous if-block
		byteCode.emit(opcode::OP_SET_INDEX);
		byteCode.emit(char(0xF4));
		byteCode.emit(short(0));

		auto elseLoc = byteCode.getBytecodePos() - 2;

		node->elseBlock->visit(this);
		byteCode.emit(short(byteCode.getOpIndex()), elseLoc);
	}
}

void GS2CompilerVisitor::Visit(ExpressionNewArrayNode *node)
{
	assert(!node->dimensions.empty());

	byteCode.emit(opcode::OP_TYPE_NUMBER);
	byteCode.emitDynamicNumber(node->dimensions[0]);
	byteCode.emit(opcode::OP_ARRAY_NEW);

	for (auto i = 1; i < node->dimensions.size(); i++)
	{
		byteCode.emit(opcode::OP_TYPE_NUMBER);
		byteCode.emitDynamicNumber(node->dimensions[i]);
		byteCode.emit(opcode::OP_ARRAY_NEW_MULTIDIM);
	}
}

void GS2CompilerVisitor::Visit(ExpressionNewObjectNode *node)
{
	// TODO(joey): more testing needed
	// temp.a = new TStaticVar("str") will return a regular string,
	// but if there is additional args it has no effect on the output.

	auto identNode = reinterpret_cast<ExpressionIdentifierNode*>(node->newExpr);
	auto identIdx = byteCode.getStringConst(*identNode->val);

	// new only works with one argument, and the argument is the object name
	if (node->args.size() == 1)
	{
		node->args.front()->visit(this);
		byteCode.emit(opcode::OP_INLINE_NEW);
	}
	else
	{
		byteCode.emit(opcode::OP_TYPE_VAR);
		byteCode.emitDynamicNumberUnsigned(byteCode.getStringConst("unknown_object"));
	}

	byteCode.emit(opcode::OP_TYPE_STRING);
	byteCode.emitDynamicNumberUnsigned(identIdx);

	byteCode.emit(opcode::OP_NEW_OBJECT);
}

void GS2CompilerVisitor::Visit(StatementWhileNode *node)
{
	ScopeGuard guard(*this);

	JumpTarget breakJmp(byteCode);
	JumpTarget continueJmp(byteCode);

	break_target = &breakJmp;
	continue_target = &continueJmp;

	// Set the continue breakpoint to the start of the loop
	continueJmp.resolveHere();

	{
		_isInlineConditional = false;
		node->expr->visit(this);
		_isInlineConditional = true;
	}

	byteCode.emitConversionOp(node->expr->expressionType(), ExpressionType::EXPR_NUMBER);

	breakJmp.emitJump(opcode::OP_IF);

	// Increment loop count
	byteCode.emit(opcode::OP_CMD_CALL);

	node->block->visit(this);

	// Jump back to condition
	continueJmp.emitJump(opcode::OP_SET_INDEX);

	// Set the fail breakpoint to after the while-statement
	breakJmp.resolveHere();
}

void GS2CompilerVisitor::Visit(StatementBreakNode* node)
{
	if (!break_target)
	{
		std::string errorMsg = std::format("`break` outside loop detected");
		parserContext.addError({ ErrorLevel::E_WARNING, GS2CompilerError::ErrorCategory::Compiler, std::move(errorMsg) });
		return;
	}

	// Emit jump out of loop
	break_target->emitJump(opcode::OP_SET_INDEX);
}

void GS2CompilerVisitor::Visit(StatementContinueNode* node)
{
	if (!continue_target)
	{
		std::string errorMsg = std::format("`continue` outside loop detected");
		parserContext.addError({ ErrorLevel::E_WARNING, GS2CompilerError::ErrorCategory::Compiler, std::move(errorMsg) });
		return;
	}

	// Emit jump back to the loop-condition
	continue_target->emitJump(opcode::OP_SET_INDEX);
}

void GS2CompilerVisitor::Visit(StatementForNode *node)
{
	// Emit init expression
	if (node->init)
		node->init->visit(this);

	// Start of loop
	auto startLoopOp = byteCode.getOpIndex();

	// Emit the condition expression
	if (node->cond)
	{
		node->cond->visit(this);
		byteCode.emitConversionOp(node->cond->expressionType(), ExpressionType::EXPR_NUMBER);
	}
	else
	{
		// No condition, so while (true)
		// note: parser should throw a syntax error so this should never really occur.
		byteCode.emit(opcode::OP_TYPE_TRUE);
	}

	ScopeGuard guard(*this);

	JumpTarget breakJmp(byteCode);
	JumpTarget continueJmp(byteCode);

	break_target = &breakJmp;
	continue_target = &continueJmp;

	// Emit if-loop on conditional expression, with a failed jump to the end-block
	breakJmp.emitJump(opcode::OP_IF);

	// Increment loop count
	byteCode.emit(opcode::OP_CMD_CALL);

	// Emit block
	if (node->block)
		node->block->visit(this);

	// Set the continue location before the post-op
	continueJmp.resolveHere();

	// Emit post-op
	if (node->postop)
	{
		// TODO(joey): discard return
		node->postop->visit(this);
	}

	// Emit jump back to condition
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emitDynamicNumber(startLoopOp);

	// Write out the breakpoint jumps
	breakJmp.resolveHere();
}

void GS2CompilerVisitor::Visit(StatementNewNode* node)
{
	assert(node->args.size() == 1);

	// emit args
	for (const auto& n : node->args)
		n->visit(this);

	byteCode.emit(opcode::OP_INLINE_NEW);

	byteCode.emit(opcode::OP_COPY_LAST_OP);
	byteCode.emit(opcode::OP_COPY_LAST_OP);
	byteCode.emit(opcode::OP_COPY_LAST_OP);

	// emit object type
	auto id = byteCode.getStringConst(*node->ident);
	byteCode.emit(opcode::OP_TYPE_STRING);
	byteCode.emitDynamicNumberUnsigned(id);

	// official emits this
	byteCode.emit(opcode::OP_CONV_TO_STRING);

	byteCode.emit(opcode::OP_NEW_OBJECT);
	byteCode.emit(opcode::OP_ASSIGN);

	// with statement
	byteCode.emit(opcode::OP_CONV_TO_OBJECT);

	byteCode.emit(opcode::OP_WITH);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));

	auto withLoc = byteCode.getBytecodePos() - 2;

	int prevNewObjectCount = _newObjectCount++;
	if (node->stmtBlock)
		node->stmtBlock->visit(this);

	byteCode.emit(opcode::OP_WITHEND);
	byteCode.emit(short(byteCode.getOpIndex()), withLoc);

	///////
	// call addcontrol
	for (int i = 0; i < _newObjectCount - prevNewObjectCount; i++)
	{
		byteCode.emit(opcode::OP_TYPE_ARRAY);
		byteCode.emit(opcode::OP_SWAP_LAST_OPS);

		auto addControlId = byteCode.getStringConst("addcontrol");
		byteCode.emit(opcode::OP_TYPE_VAR);
		byteCode.emitDynamicNumberUnsigned(addControlId);
		byteCode.emit(opcode::OP_CALL);
		byteCode.emit(opcode::OP_INDEX_DEC);
	}

	_newObjectCount--;
}

void GS2CompilerVisitor::Visit(StatementWithNode* node)
{
	node->expr->visit(this);
	byteCode.emit(opcode::OP_CONV_TO_OBJECT);

	byteCode.emit(opcode::OP_WITH);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));

	auto withLoc = byteCode.getBytecodePos() - 2;
	if (node->block)
		node->block->visit(this);

	byteCode.emit(opcode::OP_WITHEND);
	byteCode.emit(short(byteCode.getOpIndex()), withLoc);
}

void GS2CompilerVisitor::Visit(ExpressionListNode* node)
{
	byteCode.emit(opcode::OP_TYPE_ARRAY);

	for (auto it = node->args.rbegin(); it != node->args.rend(); ++it)
	{
		assert(*it);
		(*it)->visit(this);
	}

	byteCode.emit(opcode::OP_ARRAY_END);
}

void GS2CompilerVisitor::Visit(StatementForEachNode *node)
{
	// push name / expression
	node->name->visit(this);
	node->expr->visit(this);
	byteCode.emit(opcode::OP_CONV_TO_OBJECT);

	// push index to stack
	byteCode.emit(opcode::OP_TYPE_NUMBER);
	byteCode.emitDynamicNumber(0);

	ScopeGuard guard(*this);

	JumpTarget breakJmp(byteCode);
	JumpTarget continueJmp(byteCode);

	break_target = &breakJmp;
	continue_target = &continueJmp;

	auto startLoopOp = byteCode.getOpIndex();
	breakJmp.emitJump(opcode::OP_FOREACH);

	byteCode.emit(opcode::OP_CMD_CALL);
	node->block->visit(this);

	// Set the continue location before we increment the idx
	continueJmp.resolveHere();
	byteCode.emit(opcode::OP_INC);

	// jump to beginning of the for-each loop
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emitDynamicNumber(startLoopOp);

	// Write out the breakpoint jumps
	breakJmp.resolveHere();

	// pop the idx variable (after guard restores targets)
	byteCode.emit(opcode::OP_INDEX_DEC);
}

void GS2CompilerVisitor::Visit(StatementSwitchNode* node)
{
	// emit jump to case-test
	// case-list:
	// record case-block start
	// emit case-block
	// emit jump to endloc // actually no
	// ...repeat..

	// case-test:
	// push switch-expr
	// copy last operand
	// push case-expr
	// push ==
	// opcode 2 if equal, jmp to corresponding case-block

	// endloc:
	// ....

	ScopeGuard guard(*this);

	JumpTarget breakJmp(byteCode);

	std::vector<jmp_address> caseAddrs;

	// jump to case-test
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));
	size_t caseTestLoc = byteCode.getBytecodePos() - 2;

	// case-list:
	for (const auto& caseNode : node->cases)
	{
		auto caseAddr = byteCode.getOpIndex();

		for (const auto& caseExpr : caseNode.exprList)
			caseAddrs.push_back(caseAddr);

		break_target = &breakJmp;

		// continue inside switch case jumps to current case start
		JumpTarget caseStart(byteCode);
		caseStart.resolve(caseAddr);
		continue_target = &caseStart;

		caseNode.block->visit(this);
		continue_target = nullptr;
	}

	// case-test:
	byteCode.emit(short(byteCode.getOpIndex()), caseTestLoc);
	node->expr->visit(this);

	size_t i = 0;
	for (const auto& caseNode : node->cases)
	{
		for (const auto& caseExpr : caseNode.exprList)
		{
			if (caseExpr)
			{
				byteCode.emit(opcode::OP_COPY_LAST_OP);
				caseExpr->visit(this);
				byteCode.emit(opcode::OP_EQ);
				byteCode.emit(opcode::OP_SET_INDEX_TRUE);
			}
			else byteCode.emit(opcode::OP_SET_INDEX);

			byteCode.emitDynamicNumber(caseAddrs[i++]);
		}
	}

	breakJmp.resolveHere();

	// Since we are consuming a copy for each case-test, we need to pop
	// the original value off the top of the stack.
	byteCode.emit(opcode::OP_INDEX_DEC);
}

// not implemented: should never occur
void GS2CompilerVisitor::Visit(StatementNode *node) { Visit((Node *)node); }
void GS2CompilerVisitor::Visit(ExpressionNode *node) { Visit((Node *)node); }
