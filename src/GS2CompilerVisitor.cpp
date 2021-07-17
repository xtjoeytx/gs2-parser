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
	{"length", opcode::OP_OBJ_LENGTH, false},
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
	
	byteCode.addFunction({node->ident, byteCode.getBytecodePos(), byteCode.getOpcodePos()});
	
	{
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
		case ExpressionOp::LessThan: return opcode::Opcode::OP_LT;
		case ExpressionOp::LessThanOrEqual: return opcode::Opcode::OP_LTE;
		case ExpressionOp::GreaterThan: return opcode::Opcode::OP_GT;
		case ExpressionOp::GreaterThanOrEqual: return opcode::Opcode::OP_GTE;

		default: return opcode::Opcode::OP_NUM_OPS;
	}
}

void GS2CompilerVisitor::Visit(ExpressionBinaryOpNode *node)
{
	bool handled = false;

	if (node->op == ExpressionOp::Concat)
	{
		node->left->visit(this);
		if (node->left->expressionType() != ExpressionType::EXPR_STRING)
			byteCode.emit(opcode::OP_CONV_TO_STRING);

		node->right->visit(this);
		if (node->right->expressionType() != ExpressionType::EXPR_STRING)
			byteCode.emit(opcode::OP_CONV_TO_STRING);

		byteCode.emit(opcode::OP_JOIN);
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
			assert(opCode != opcode::Opcode::OP_NUM_OPS);

			byteCode.emit(opCode);
			handled = true;
			break;
		}

		case ExpressionOp::Assign:
		case ExpressionOp::Equal:
		{
			node->left->visit(this);
			node->right->visit(this);

			auto opCode = getExpressionOpCode(node->op);
			assert(opCode != opcode::Opcode::OP_NUM_OPS);

			byteCode.emit(opCode);
			handled = true;
			break;
		}
	}

	////////
	if (!handled)
	{
		Visit((Node*)node);
	}
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
	}
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

void GS2CompilerVisitor::Visit(ExpressionObjectAccessNode *node)
{
	// this -> 180
	// thiso -> 181
	// player -> 182
	// playero -> 183
	// level -> 184
	// temp -> 189
	// params[] -> 190
	auto firstNode = node->left->toString();
	if (firstNode == "this") {
		byteCode.emit(opcode::OP_THIS);
	}
	else if (firstNode == "thiso") {
		byteCode.emit(opcode::OP_THISO);
	}
	else if (firstNode == "player") {
		byteCode.emit(opcode::OP_PLAYER);
	}
	else if (firstNode == "playero") {
		byteCode.emit(opcode::OP_PLAYERO);
	}
	else if (firstNode == "level") {
		byteCode.emit(opcode::OP_LEVEL);
	}
	else if (firstNode == "temp") {
		byteCode.emit(opcode::OP_TEMP);
	}
	else {
		node->left->visit(this);
		if (node->left->expressionType() != ExpressionType::EXPR_OBJECT)
			byteCode.emit(opcode::OP_CONV_TO_OBJECT);
	}

	for (auto i = 0; i < node->nodes.size(); i++)
	{
		node->nodes[i]->visit(this);
		byteCode.emit(opcode::OP_MEMBER_ACCESS);
		if (i != (node->nodes.size() - 1))
			byteCode.emit(opcode::OP_CONV_TO_OBJECT);
	}

	if (node->right)
	{
		node->right->visit(this);
		byteCode.emit(opcode::OP_MEMBER_ACCESS);
	}

	//node->left->visit(this);
	//node->right->visit(this);

	//printf("Test left: %s\n", node->left->toString().c_str());
	//printf("Test right: %s\n", node->right->toString().c_str());
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

void GS2CompilerVisitor::Visit(ExpressionStringConstNode *node)
{
	printf("String: %s\n", node->val.c_str());

	auto id = byteCode.getStringConst(node->val);
	
	byteCode.emit(opcode::OP_TYPE_STRING);
	byteCode.emitDynamicNumber(id);
//	byteCode.emit((char)0xF0);
//	byteCode.emit((char)id);
}

void GS2CompilerVisitor::Visit(ExpressionFnCallNode *node)
{
	// Build-in commands
	auto& cmdList = (node->objExpr ? getCommandListMethod() : getCommandList());
	std::string funcName = node->funcExpr->toString();

	printf("Call Function: %s\n", funcName.c_str());

	auto iter = cmdList.find(funcName);
	BuiltInCmd cmd = (iter != cmdList.end() ? iter->second : defaultCall);
	
	if (node->objExpr)
		node->objExpr->visit(this);

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

		if (cmd.op == opcode::OP_CALL)
			node->funcExpr->visit(this);

		byteCode.emit(cmd.op);
	}

	if (node->discardReturnValue)
		byteCode.emit(opcode::OP_INDEX_DEC);
}

void GS2CompilerVisitor::Visit(StatementReturnNode *node)
{
	// pretty sure it goes on top
	if (node->expr)
		node->expr->visit(this);

	//byteCode.emit(opcode::OP_CONV_TO_STRING);
	byteCode.emit(opcode::OP_RET);
}

void GS2CompilerVisitor::Visit(StatementIfNode* node)
{
	node->expr->visit(this);
	if (node->expr->expressionType() != ExpressionType::EXPR_INTEGER)
		byteCode.emit(opcode::OP_CONV_TO_FLOAT);

	byteCode.emit(opcode::OP_IF);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0));
	auto ifLoc = byteCode.getBytecodePos() - 2;

	node->thenBlock->visit(this);

	// OP_IF jumps to this location if the condition is false, so we
	// continue to the next instruction, but if their is an else-block we must
	// skip the next instruction since its a jmp to the end of the if-else chain
	auto nextOpcode = byteCode.getOpcodePos() + (node->elseBlock ? 1 : 0);
	byteCode.emit(short(nextOpcode), ifLoc);

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

void GS2CompilerVisitor::Visit(StatementNewNode *node)
{
	Visit((Node*)node);
}

void GS2CompilerVisitor::Visit(ExpressionNewNode *node)
{
	// TODO(joey): more testing needed
	// temp.a = new TStaticVar("str") will return a regular string,
	// but if there is additional args it has no effect on the output.
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

	breakPoints.push(LoopBreakPoint{ whileCondStart });
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
	for (const auto& loc : breakPoint.breakPointLocs) {
		byteCode.emit(short(breakPointLoc), loc);
	}
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
	byteCode.emitDynamicNumber(breakPoints.top().continuepoint);
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

	breakPoints.push(LoopBreakPoint{ condStart });

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
	for (const auto& loc : breakPoint.breakPointLocs) {
		byteCode.emit(short(breakPointLoc), loc);
	}
	breakPoints.pop();
}

void GS2CompilerVisitor::Visit(StatementNode *node) { Visit((Node *)node); }
void GS2CompilerVisitor::Visit(StatementForEachNode *node) { Visit((Node *)node); }
void GS2CompilerVisitor::Visit(StatementSwitchNode *node) { Visit((Node *)node); }
void GS2CompilerVisitor::Visit(StatementWithNode *node) { Visit((Node *)node); }
void GS2CompilerVisitor::Visit(ExpressionNode *node) { Visit((Node *)node); }
void GS2CompilerVisitor::Visit(ExpressionUnaryOpNode *node) { Visit((Node *)node); }
void GS2CompilerVisitor::Visit(ExpressionListNode *node) { Visit((Node *)node); }