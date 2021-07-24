#include <cassert>
#include <unordered_map>
#include "GS2CompilerVisitor.h"
#include "ast.h"
#include "Parser.h"

struct BuiltInCmd
{
	std::string name;
	opcode::Opcode op;
	bool useArray;
	opcode::Opcode convert_op{ opcode::OP_CONV_TO_OBJECT };
};

const BuiltInCmd defaultCall = {
	"", opcode::OP_CALL, true
};

const BuiltInCmd builtInCmds[] = {
	{"makevar", opcode::OP_MAKEVAR, true},
	{"format", opcode::OP_FORMAT, true},
	{"abs", opcode::OP_ABS, false},
	{"random", opcode::OP_RANDOM, false},
	{"sin", opcode::OP_SIN, false},
	{"cos", opcode::OP_COS, false},

	{"arctan", opcode::OP_ARCTAN, false},
	{"exp", opcode::OP_EXP, false},
	{"log", opcode::OP_LOG, false},
	{"min", opcode::OP_MIN, false},
	{"max", opcode::OP_MAX, false},
	{"getangle", opcode::OP_GETANGLE, false},
	{"getdir", opcode::OP_GETDIR, false},
	{"vecx", opcode::OP_VECX, false},
	{"vecy", opcode::OP_VECY, false},
};

const BuiltInCmd builtInObjCmds[] = {
	{"index", opcode::OP_OBJ_INDEX, false},
	{"type", opcode::OP_OBJ_TYPE, false},

	//
	{"indices", opcode::OP_OBJ_INDICES, false},
	{"link", opcode::OP_OBJ_LINK, false},
	{"trim", opcode::OP_OBJ_TRIM, false},
	{"length", opcode::OP_OBJ_LENGTH, false, opcode::OP_CONV_TO_STRING},
	{"pos", opcode::OP_OBJ_POS, false},
	{"charat", opcode::OP_OBJ_CHARAT, false},
	{"substring", opcode::OP_OBJ_SUBSTR, false},
	{"starts", opcode::OP_OBJ_STARTS, false},
	{"ends", opcode::OP_OBJ_ENDS, false},
	{"tokenize", opcode::OP_OBJ_TOKENIZE, false},
	{"positions", opcode::OP_OBJ_POSITIONS, false},
	{"size", opcode::OP_OBJ_SIZE, false},
	{"subarray", opcode::OP_OBJ_SUBARRAY, false},
	{"clear", opcode::OP_OBJ_CLEAR, false},
};

const std::unordered_map<std::string, BuiltInCmd>& getCommandList()
{
	static bool initialized = false;
	static std::unordered_map<std::string, BuiltInCmd> builtInMap;

	if (!initialized)
	{
		for (const auto& cmd : builtInCmds)
			builtInMap.insert({ cmd.name, cmd });

		initialized = true;
	}

	return builtInMap;
}

const std::unordered_map<std::string, BuiltInCmd>& getCommandListMethod()
{
	static bool initialized = false;
	static std::unordered_map<std::string, BuiltInCmd> builtInMap;

	if (!initialized)
	{
		for (const auto& cmd : builtInObjCmds)
			builtInMap.insert({ cmd.name, cmd });

		initialized = true;
	}

	return builtInMap;
}

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
		case ExpressionOp::Assign: return opcode::Opcode::OP_ASSIGN;
		case ExpressionOp::Equal: return opcode::Opcode::OP_EQ;
		case ExpressionOp::NotEqual: return opcode::Opcode::OP_NEQ;
		case ExpressionOp::LessThan: return opcode::Opcode::OP_LT;
		case ExpressionOp::LessThanOrEqual: return opcode::Opcode::OP_LTE;
		case ExpressionOp::GreaterThan: return opcode::Opcode::OP_GT;
		case ExpressionOp::GreaterThanOrEqual: return opcode::Opcode::OP_GTE;

		case ExpressionOp::UnaryMinus: return opcode::Opcode::OP_UNARYSUB;
		case ExpressionOp::UnaryNot: return opcode::Opcode::OP_NOT;
		case ExpressionOp::Increment: return opcode::Opcode::OP_INC;
		case ExpressionOp::Decrement: return opcode::Opcode::OP_DEC;

		default: return opcode::Opcode::OP_NONE;
	}
}

void GS2CompilerVisitor::Visit(Node *node)
{
	fprintf(stderr, "Unimplemented node type: %s\n", node->NodeType());
#ifdef _WIN32
	system("pause");
#endif
	exit(1);
}

void GS2CompilerVisitor::Visit(StatementBlock *node)
{
   for (const auto& n : node->statements)
	{
		if (n)
			n->visit(this);
	}
}

void GS2CompilerVisitor::Visit(StatementFnDeclNode *node)
{
	printf("Declare function: %s\n", node->ident.c_str());

	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0)); // replaced with jump index to last opcode

	std::string funcName;
	if (node->pub)
		funcName.append("public.");
	if (!node->objectName.empty())
		funcName.append(node->objectName).append(".");
	funcName.append(node->ident);

	byteCode.addFunction({funcName, byteCode.getBytecodePos(), byteCode.getOpcodePos()});

	{
		byteCode.emit(opcode::OP_TYPE_ARRAY);

		std::reverse(node->args.begin(), node->args.end());
		for (const auto& arg : node->args)
		{
			if (arg)
				arg->visit(this);
		}

		byteCode.emit(opcode::OP_FUNC_PARAMS_END);
	}

	byteCode.emit(opcode::OP_JMP);
	byteCode.emit(opcode::OP_CMD_CALL);

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

void GS2CompilerVisitor::Visit(ExpressionBinaryOpNode *node)
{
	bool handled = false;

	if (node->op == ExpressionOp::LogicalAnd || node->op == ExpressionOp::LogicalOr)
	{
		node->left->visit(this);

		if (node->op == ExpressionOp::LogicalAnd)
		{
			byteCode.emit(opcode::OP_IF);
			byteCode.emit(char(0xF4));
			byteCode.emit(short(0));

			// TODO(joey): This is not going to work when using logical-and in expressions
			// that aren't if-statements, so back to the drawing board
			if (!logicalBreakpoints.empty()) {
				logicalBreakpoints.top().breakPointLocs.push_back(byteCode.getBytecodePos() - 2);
			}

			node->right->visit(this);
			return;
		}
		else if (node->op == ExpressionOp::LogicalOr)
		{
			//byteCode.emit(opcode::OP_OR);
			//byteCode.emit(char(0xF4));
			//byteCode.emit(short(0));

			//node->right->visit(this);
			handled = false;
		}
	}

	switch (node->op)
	{
		case ExpressionOp::Plus:
		case ExpressionOp::Minus:
		case ExpressionOp::Multiply:
		case ExpressionOp::Divide:
		case ExpressionOp::Mod:
		case ExpressionOp::Pow:
		case ExpressionOp::LessThan:
		case ExpressionOp::LessThanOrEqual:
		case ExpressionOp::GreaterThan:
		case ExpressionOp::GreaterThanOrEqual:
		{
			node->left->visit(this);
			if (node->left->expressionType() != ExpressionType::EXPR_INTEGER)
				byteCode.emit(opcode::OP_CONV_TO_FLOAT);
			node->right->visit(this);

			auto opCode = getExpressionOpCode(node->op);
			assert(opCode != opcode::Opcode::OP_NONE);

			byteCode.emit(opCode);
			handled = true;
			break;
		}

		case ExpressionOp::Equal:
		case ExpressionOp::NotEqual:
		{
			node->left->visit(this);
			node->right->visit(this);

			auto opCode = getExpressionOpCode(node->op);
			assert(opCode != opcode::Opcode::OP_NONE);

			byteCode.emit(opCode);
			handled = true;
			break;
		}

		case ExpressionOp::Assign:
		{
			node->left->visit(this);
			node->right->visit(this);

			auto opCode = getExpressionOpCode(node->op);
			assert(opCode != opcode::Opcode::OP_NONE);

			// Special assignment operators for array/multi-dimensional arrays
			auto exprType = node->left->expressionType();
			if (exprType == ExpressionType::EXPR_ARRAY)
				opCode = opcode::Opcode::OP_ARRAY_ASSIGN;
			else if (exprType == ExpressionType::EXPR_MULTIARRAY)
				opCode = opcode::Opcode::OP_ARRAY_MULTIDIM_ASSIGN;
			
			byteCode.emit(opCode);
			handled = true;
			break;
		}

		default:
			handled = false;
			break;
	}

	////////
	if (!handled)
	{
		printf("Undefined opcode: %s (%d)\n", ExpressionOpToString(node->op), node->op);
		Visit((Node*)node);
	}
}

void GS2CompilerVisitor::Visit(ExpressionUnaryOpNode* node)
{
	bool handled = false;

	node->expr->visit(this);

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

				handled = true;
				break;
			}

			case ExpressionOp::UnaryMinus:
			case ExpressionOp::UnaryNot:
			{
				auto opCode = getExpressionOpCode(node->op);
				assert(opCode != opcode::Opcode::OP_NONE);

				byteCode.emit(opcode::OP_CONV_TO_FLOAT);
				byteCode.emit(opCode);
				handled = true;
				break;
			}

			case ExpressionOp::UnaryStringCast:
			{
				byteCode.emit(opcode::OP_CONV_TO_STRING);
				handled = true;
				break;
			}

			default:
				handled = false;
				break;
		}
	}
	else
	{
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
				handled = true;
				break;
			}

			default:
				handled = false;
				break;
		}
	}

	if (!handled)
	{
		Visit((Node*)node);
	}
}

void GS2CompilerVisitor::Visit(ExpressionStrConcatNode *node)
{
	node->left->visit(this);
	if (node->left->expressionType() != ExpressionType::EXPR_STRING)
		byteCode.emit(opcode::OP_CONV_TO_STRING);

	switch (node->sep)
	{
		case ' ':
		case '\t':
		case '\n':
			auto id = byteCode.getStringConst(std::string(1, node->sep));
			byteCode.emit(opcode::OP_TYPE_STRING);
			byteCode.emitDynamicNumber(id);

			byteCode.emit(opcode::OP_JOIN);
			break;
	}
	
	node->right->visit(this);
	if (node->right->expressionType() != ExpressionType::EXPR_STRING)
		byteCode.emit(opcode::OP_CONV_TO_STRING);

	byteCode.emit(opcode::OP_JOIN);
}

void GS2CompilerVisitor::Visit(ExpressionCastNode* node)
{
	node->expr->visit(this);

	switch (node->type)
	{
		case ExpressionCastNode::CastType::INTEGER:
			byteCode.emit(opcode::OP_INT);
			break;

		case ExpressionCastNode::CastType::FLOAT:
			byteCode.emit(opcode::OP_CONV_TO_FLOAT);
			break;

		case ExpressionCastNode::CastType::STRING:
			byteCode.emit(opcode::OP_CONV_TO_STRING);
			break;
	}
}

void GS2CompilerVisitor::Visit(ExpressionArrayIndexNode* node)
{
	for (const auto& expr : node->exprList)
	{
		expr->visit(this);
		if (expr->expressionType() != ExpressionType::EXPR_NUMBER
			&& expr->expressionType() != ExpressionType::EXPR_INTEGER)
		{
			byteCode.emit(opcode::OP_CONV_TO_FLOAT);
		}
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
	node->expr->visit(this);

	node->lower->visit(this);
	if (node->lower->expressionType() != ExpressionType::EXPR_NUMBER)
		byteCode.emit(opcode::OP_CONV_TO_FLOAT);

	if (node->higher)
	{
		node->higher->visit(this);

		byteCode.emit(opcode::OP_IN_RANGE);
	}
	else
	{
		byteCode.emit(opcode::OP_IN_OBJ);
	}

	byteCode.emit(char(0xF3));
	byteCode.emit(char(0));
}

void GS2CompilerVisitor::Visit(ExpressionIdentifierNode *node)
{
	auto enumConstant = parserData->getEnumConstant(node->val);
	if (enumConstant)
	{
		byteCode.emit(opcode::OP_TYPE_NUMBER);
		byteCode.emitDynamicNumber(enumConstant.value());
		return;
	}

	auto id = byteCode.getStringConst(node->val);

	byteCode.emit(opcode::OP_TYPE_VAR);
	byteCode.emit((char)0xF0);
	byteCode.emit((char)id);

	printf("Identifier Node: %s\n", node->val.c_str());
}

void GS2CompilerVisitor::Visit(ExpressionIntegerNode *node)
{
	byteCode.emit(opcode::OP_TYPE_NUMBER);
	byteCode.emitDynamicNumber(node->val);
}

void GS2CompilerVisitor::Visit(ExpressionNumberNode *node)
{
	byteCode.emit(opcode::OP_TYPE_NUMBER);
	byteCode.emitDoubleNumber(node->val);
}

void GS2CompilerVisitor::Visit(ExpressionPostfixNode* node)
{
	assert(!node->nodes.empty());

	size_t i = 0;

	if (node->nodes[0]->expressionType() == ExpressionType::EXPR_IDENT)
	{
		auto identNode = reinterpret_cast<ExpressionIdentifierNode*>(node->nodes[0]);
		if (identNode->val == "this") {
			byteCode.emit(opcode::OP_THIS);
		}
		else if (identNode->val == "thiso") {
			byteCode.emit(opcode::OP_THISO);
		}
		else if (identNode->val == "player") {
			byteCode.emit(opcode::OP_PLAYER);
		}
		else if (identNode->val == "playero") {
			byteCode.emit(opcode::OP_PLAYERO);
		}
		else if (identNode->val == "level") {
			byteCode.emit(opcode::OP_LEVEL);
		}
		else if (identNode->val == "temp") {
			byteCode.emit(opcode::OP_TEMP);
		}
		else {
			identNode->visit(this);
			if (node->nodes.size() > 1)
				byteCode.emit(opcode::OP_CONV_TO_OBJECT);
		}
		i++;
	}
	
	// mark our last node as an assignment
	if (node->isAssignment)
		node->nodes.back()->isAssignment = true;

	for (; i < node->nodes.size(); i++)
	{
		node->nodes[i]->visit(this);
		if (i >= 1 && node->nodes[i]->expressionType() == ExpressionType::EXPR_IDENT)
		{
			byteCode.emit(opcode::OP_MEMBER_ACCESS);
			if (i != node->nodes.size() - 1)
				byteCode.emit(opcode::OP_CONV_TO_OBJECT);
		}
	}
}

void GS2CompilerVisitor::Visit(ExpressionStringConstNode *node)
{
	printf("String: %s\n", node->val.c_str());

	auto id = byteCode.getStringConst(node->val);

	byteCode.emit(opcode::OP_TYPE_STRING);
	byteCode.emitDynamicNumber(id);
}

void GS2CompilerVisitor::Visit(ExpressionFnCallNode *node)
{
	auto isObjectCall = (node->objExpr != nullptr);

	// Build-in commands
	auto& cmdList = (isObjectCall ? getCommandListMethod() : getCommandList());
	std::string funcName = node->funcExpr->toString();

	printf("Call Function: %s (obj call: %d)\n", funcName.c_str(), isObjectCall ? 1 : 0);

	auto iter = cmdList.find(funcName);
	BuiltInCmd cmd = (iter != cmdList.end() ? iter->second : defaultCall);

	{
		if (cmd.useArray)
			byteCode.emit(opcode::OP_TYPE_ARRAY);

		if (node->args)
		{
			std::reverse(node->args->begin(), node->args->end());
			for (auto& arg : *node->args)
			{
				if (arg)
					arg->visit(this);
			}
		}

		if (isObjectCall) {
			node->objExpr->visit(this);
			byteCode.emit(cmd.convert_op);
		}

		if (cmd.op == opcode::OP_CALL)
		{
			node->funcExpr->visit(this);

			if (isObjectCall)
				byteCode.emit(opcode::OP_MEMBER_ACCESS);
		}

		byteCode.emit(cmd.op);
	}

	if (node->discardReturnValue)
		byteCode.emit(opcode::OP_INDEX_DEC);
}

void GS2CompilerVisitor::Visit(StatementReturnNode *node)
{
	if (node->expr)
		node->expr->visit(this);
	else
	{
		byteCode.emit(opcode::OP_TYPE_NUMBER);
		byteCode.emitDynamicNumber(0);
	}

	byteCode.emit(opcode::OP_RET);
}

void GS2CompilerVisitor::Visit(StatementIfNode* node)
{
	node->expr->visit(this);
	if (node->expr->expressionType() != ExpressionType::EXPR_INTEGER)
		byteCode.emit(opcode::OP_CONV_TO_FLOAT);

	logicalBreakpoints.push(LogicalBreakPoint{ byteCode.getOpcodePos() });

	byteCode.emit(opcode::OP_IF);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));

	logicalBreakpoints.top().breakPointLocs.push_back(byteCode.getBytecodePos() - 2);

	node->thenBlock->visit(this);

	// OP_IF jumps to this location if the condition is false, so we
	// continue to the next instruction, but if their is an else-block we must
	// skip the next instruction since its a jmp to the end of the if-else chain
	auto nextOpcode = byteCode.getOpcodePos() + (node->elseBlock ? 1 : 0);

	auto& breakPoint = logicalBreakpoints.top();
	for (const auto& loc : breakPoint.breakPointLocs) {
		byteCode.emit(short(nextOpcode), loc);
	}
	logicalBreakpoints.pop();

	if (node->elseBlock)
	{
		// emit a jump to the end of this else block for the previous if-block
		byteCode.emit(opcode::OP_SET_INDEX);
		byteCode.emit(char(0xF4));
		byteCode.emit(short(0));

		auto elseLoc = byteCode.getBytecodePos() - 2;

		node->elseBlock->visit(this);
		byteCode.emit(short(byteCode.getOpcodePos()), elseLoc);
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
	
	if (node->args)
	{
		for (const auto& n : *node->args)
			n->visit(this);
	}

	// TODO(joey): fix

	auto identNode = reinterpret_cast<ExpressionIdentifierNode*>(node->newExpr);
	auto id = byteCode.getStringConst(identNode->val);

	if (identNode->val == "TStaticVar") {
		byteCode.emit(opcode::OP_TYPE_VAR);
		byteCode.emitDynamicNumber(byteCode.getStringConst("unknown_object"));
	}
	else
	{
		byteCode.emit(opcode::OP_INLINE_NEW);
	}

	byteCode.emit(opcode::OP_TYPE_STRING);
	byteCode.emitDynamicNumber(id);

	//node->newExpr->visit(this);

	byteCode.emit(opcode::OP_NEW_OBJECT);
}

void GS2CompilerVisitor::Visit(StatementWhileNode *node)
{
	auto whileCondStart = byteCode.getOpcodePos();

	node->expr->visit(this);
	if (node->expr->expressionType() != ExpressionType::EXPR_INTEGER)
		byteCode.emit(opcode::OP_CONV_TO_FLOAT);

	byteCode.emit(opcode::OP_IF);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));

	auto whileLoc = byteCode.getBytecodePos() - 2;

	breakPoints.push(LoopBreakPoint{ });
	node->block->visit(this);

	// Jump back to condition
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(whileCondStart));

	// Emit jump out of the while-loop
	auto breakPointLoc = byteCode.getOpcodePos();
	byteCode.emit(short(breakPointLoc), whileLoc);

	// Write out the breakpoint jumps
	auto& breakPoint = breakPoints.top();
	for (const auto& loc : breakPoint.breakPointLocs)
		byteCode.emit(short(breakPointLoc), loc);

	for (const auto& loc : breakPoint.continuePointLocs)
		byteCode.emit(short(whileCondStart), loc);

	breakPoints.pop();
}

void GS2CompilerVisitor::Visit(StatementBreakNode* node)
{
	if (breakPoints.empty()) {
		printf("Error, no loops to break from.\n");
		return;
	}

	// Emit jump out of loop
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));

	// Add the location of the jmp so we can write the calculated opcode index
	auto& breakPoint = breakPoints.top();
	breakPoint.breakPointLocs.push_back(byteCode.getBytecodePos() - 2);
}

void GS2CompilerVisitor::Visit(StatementContinueNode* node)
{
	if (breakPoints.empty()) {
		printf("Error, no loops to continue.\n");
		return;
	}

	// Emit jump back to the loop-condition
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));

	// Add the location of the jmp so we can write the calculated opcode index
	auto& breakPoint = breakPoints.top();
	breakPoint.continuePointLocs.push_back(byteCode.getBytecodePos() - 2);
}

void GS2CompilerVisitor::Visit(StatementForNode* node)
{
	// Emit init expression
	if (node->init) {
		node->init->visit(this);
	}

	// Emit for-loop condition
	auto condStart = byteCode.getOpcodePos();
	if (node->cond)
	{
		node->cond->visit(this);
		if (node->cond->expressionType() != ExpressionType::EXPR_INTEGER)
			byteCode.emit(opcode::OP_CONV_TO_FLOAT);
	}
	else
	{
		// Just emit 1 for the condition
		byteCode.emit(opcode::OP_TYPE_NUMBER);
		byteCode.emitDynamicNumber(0);
	}

	// Emit if-loop on conditional expression, with a failed jump to the end-block
	byteCode.emit(opcode::OP_IF);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));

	auto elseLoc = byteCode.getBytecodePos() - 2;

	breakPoints.push(LoopBreakPoint{ });

	// Emit block
	if (node->block)
		node->block->visit(this);

	// Emit post-op
	if (node->postop)
		node->postop->visit(this);

	// Emit jump back to condition
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emitDynamicNumber(condStart);

	// Emit jump out of the loop
	auto breakPointLoc = byteCode.getOpcodePos();
	byteCode.emit(short(breakPointLoc), elseLoc);

	// Write out the breakpoint jumps
	auto& breakPoint = breakPoints.top();
	for (const auto& loc : breakPoint.breakPointLocs)
		byteCode.emit(short(breakPointLoc), loc);

	for (const auto& loc : breakPoint.continuePointLocs)
		byteCode.emit(short(condStart), loc);

	breakPoints.pop();
}

//////////// not implemented yet

void GS2CompilerVisitor::Visit(StatementNewNode* node)
{
	assert(node->args && node->args->size() == 1);

	// emit args
	if (node->args)
	{
		for (const auto& n : *node->args)
			n->visit(this);
	}

	byteCode.emit(opcode::OP_INLINE_NEW);

	byteCode.emit(opcode::OP_COPY_LAST_OP);
	byteCode.emit(opcode::OP_COPY_LAST_OP);
	byteCode.emit(opcode::OP_COPY_LAST_OP);

	// emit object type
	auto id = byteCode.getStringConst(node->ident);
	byteCode.emit(opcode::OP_TYPE_STRING);
	byteCode.emitDynamicNumber(id);

	//byteCode.emit(opcode::OP_CONV_TO_STRING);

	byteCode.emit(opcode::OP_NEW_OBJECT);
	byteCode.emit(opcode::OP_ASSIGN);

	// with statement
	byteCode.emit(opcode::OP_CONV_TO_OBJECT);

	byteCode.emit(opcode::OP_WITH);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));

	auto withLoc = byteCode.getBytecodePos() - 2;
	if (node->stmtBlock)
		node->stmtBlock->visit(this);

	byteCode.emit(opcode::OP_WITHEND);
	byteCode.emit(short(byteCode.getOpcodePos()), withLoc);

	///////
	// call addcontrol

	byteCode.emit(opcode::OP_TYPE_ARRAY);
	byteCode.emit(opcode::OP_SWAP_LAST_OPS);

	auto addControlId = byteCode.getStringConst("addcontrol");
	byteCode.emit(opcode::OP_TYPE_VAR);
	byteCode.emitDynamicNumber(addControlId);
	byteCode.emit(opcode::OP_CALL);
	byteCode.emit(opcode::OP_INDEX_DEC);
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
	byteCode.emit(short(byteCode.getOpcodePos()), withLoc);
}

void GS2CompilerVisitor::Visit(ExpressionListNode* node)
{
	byteCode.emit(opcode::OP_TYPE_ARRAY);

	std::reverse(node->args.begin(), node->args.end());
	for (const auto& arg : node->args)
		arg->visit(this);

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

	auto startLoopOp = byteCode.getOpcodePos();
	byteCode.emit(opcode::OP_FOREACH);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));

	auto endLoc = byteCode.getBytecodePos() - 2;
	breakPoints.push(LoopBreakPoint{ });

	byteCode.emit(opcode::OP_CMD_CALL);
	node->block->visit(this);

	// increase idx
	auto continueLoopOp = byteCode.getOpcodePos();
	byteCode.emit(opcode::OP_INC);

	// jump to beginning of the for-each loop
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emitDynamicNumber(startLoopOp);

	// Emit jump out of the loop
	auto endLoopOp = byteCode.getOpcodePos();
	byteCode.emit(short(endLoopOp), endLoc);

	// Write out the breakpoint jumps
	auto& breakPoint = breakPoints.top();
	for (const auto& loc : breakPoint.breakPointLocs)
		byteCode.emit(short(endLoopOp), loc);

	for (const auto& loc : breakPoint.continuePointLocs)
		byteCode.emit(short(continueLoopOp), loc);

	breakPoints.pop();

	// pop index
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

	std::vector<size_t> caseStartOp;
	breakPoints.push(LoopBreakPoint{ });

	// jump to case-test
	byteCode.emit(opcode::OP_SET_INDEX);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));
	auto caseTestLoc = byteCode.getBytecodePos() - 2;

	// case-list:
	for (const auto& caseNode : node->cases)
	{
		for (const auto& caseExpr : caseNode.exprList)
			caseStartOp.push_back(byteCode.getOpcodePos());
		caseNode.block->visit(this);
	}

	// case-test:
	byteCode.emit(short(byteCode.getOpcodePos()), caseTestLoc);
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

			byteCode.emitDynamicNumber(caseStartOp[i++]);
		}
	}

	auto endLoopOp = byteCode.getOpcodePos();

	// Write out the breakpoint jumps
	auto& breakPoint = breakPoints.top();
	for (const auto& loc : breakPoint.breakPointLocs)
		byteCode.emit(short(endLoopOp), loc);

	for (const auto& loc : breakPoint.continuePointLocs)
		byteCode.emit(short(endLoopOp), loc);

	breakPoints.pop();
}

void GS2CompilerVisitor::Visit(StatementNode *node) { Visit((Node *)node); }
void GS2CompilerVisitor::Visit(ExpressionNode *node) { Visit((Node *)node); }
