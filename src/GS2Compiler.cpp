#include <unordered_map>
#include "GS2Compiler.h"
#include "ast.h"

void writeByteIntegerCode(GS2Bytecode& bc, char a)
{
	bc.emit(char(0xF3));
	bc.emit(a);
}

void GS2Compiler::Visit(Node *node)
{
	fprintf(stderr, "Unimplemented node type: %s\n", node->NodeType());
#ifdef _WIN32
	system("pause");
#endif
	exit(1);
}

void GS2Compiler::Visit(StatementBlock *node)
{
   for (const auto& n : node->statements)
	{
		if (n)
			n->visit(this);
	}
}

void GS2Compiler::Visit(StatementFnDeclNode *node)
{
	printf("Declare function: %s\n", node->ident.c_str());

	byteCode.emit(opcode::OP_SET_INDEX);
	//writeByteIntegerCode(byteCode, 0);
	byteCode.emit(char(0xF4));
	byteCode.emit(short(0)); // replaced with jump index to last opcode
	
	byteCode.addFunction({node->ident, byteCode.getBytecodePos(), byteCode.getOpcodePos()});
	//size_t jumpIdx = byteCode.getBytecodePos() - 2;
	
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
		// TODO: unsure if this is needed
		byteCode.emit(opcode::OP_TYPE_NUMBER);
		byteCode.emit(char(0xF3));
		byteCode.emit(char(0));

		byteCode.emit(opcode::OP_RET);
	}

	// this code would jump over the function definition block,
	// but it seems graal jumps to the last possible opcode+1
	// byteCode.emit(short(byteCode.getBytecodePos()), jumpIdx);
}

void GS2Compiler::Visit(ExpressionBinaryOpNode *node)
{
	bool handled = false;

	if (node->op == "@")
	{
		node->left->visit(this);
		if (node->left->expressionType() != ExpressionType::EXPR_STRING)
			byteCode.emit(opcode::OP_CONV_TO_STRING);

		node->right->visit(this);
		if (node->right->expressionType() != ExpressionType::EXPR_STRING)
			byteCode.emit(opcode::OP_CONV_TO_STRING);

		byteCode.emit(opcode::OP_JOIN);
		handled = true;
	}
	else
	{
		static const char* validOps[] = {
			"+",
			"-",
			"/",
			"*",
			"%"
		};

		static opcode::Opcode opType[] = {
			opcode::OP_ADD,
			opcode::OP_SUB,
			opcode::OP_DIV,
			opcode::OP_MUL,
			opcode::OP_MOD
		};

		bool valid = false;
		int idx = 0;
		for (const auto& opTest : validOps)
		{
			if (node->op == opTest)
			{
				valid = true;
				break;
			}
			idx++;
		}

		if (valid)
		{
			node->left->visit(this);
			node->right->visit(this);
			byteCode.emit(opType[idx]);
			handled = true;
		}
	}

	if (!handled)
	{
		Visit((Node*)node);
	}
}

void GS2Compiler::Visit(ExpressionIdentifierNode *node)
{
	auto id = byteCode.getStringConst(node->val);

	byteCode.emit(opcode::OP_TYPE_VAR);
	byteCode.emit((char)0xF0);
	byteCode.emit((char)id);

	printf("Val: %s\n", node->val.c_str());
}

void GS2Compiler::Visit(ExpressionObjectAccessNode *node)
{
	node->left->visit(this);
	node->right->visit(this);

	byteCode.emit(opcode::OP_MEMBER_ACCESS);

	printf("Test left: %s\n", node->left->toString().c_str());
	printf("Test right: %s\n", node->right->toString().c_str());
}

void GS2Compiler::Visit(ExpressionIntegerNode *node)
{
	byteCode.emit(opcode::OP_TYPE_NUMBER);
	byteCode.emit((char)0xF3);
	byteCode.emit((char)node->val); // TODO
}

void GS2Compiler::Visit(ExpressionStringConstNode *node)
{
	auto id = byteCode.getStringConst(node->val);
	printf("String: %s\n", node->val.c_str());

	byteCode.emit(opcode::OP_TYPE_STRING);
	byteCode.emit((char)0xF0);
	byteCode.emit((char)id);
}

void GS2Compiler::Visit(ExpressionFnCallNode *node)
{
	printf("Call: %s\n", node->expr->toString().c_str());

	// Build-in commands
	static const std::unordered_map<std::string, opcode::Opcode> builtInCmds = {
		{"format", opcode::Opcode::OP_FORMAT},
		{"int", opcode::Opcode::OP_INT},
		{"abs", opcode::Opcode::OP_ABS},
		{"random", opcode::Opcode::OP_RANDOM},
		{"sin", opcode::Opcode::OP_SIN},
		{"cos", opcode::Opcode::OP_COS},

		{"arctan", opcode::Opcode::OP_ARCTAN},
		{"exp", opcode::Opcode::OP_EXP},
		{"log", opcode::Opcode::OP_LOG},
		{"min", opcode::Opcode::OP_MIN},
		{"max", opcode::Opcode::OP_MAX},
		{"getangle", opcode::Opcode::OP_GETANGLE},
		{"getdir", opcode::Opcode::OP_GETDIR},
		{"vecx", opcode::Opcode::OP_VECX},
		{"vecy", opcode::Opcode::OP_VECY},
	};
	
	opcode::Opcode opCode = opcode::OP_CALL;

	std::string cmdName = node->expr->toString();
	auto it = builtInCmds.find(cmdName);
	if (it != builtInCmds.end())
		opCode = it->second;
	
	{
		if (opCode != opcode::OP_MAX && opCode != opcode::OP_MIN)
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

		if (opCode == opcode::OP_CALL)
			node->expr->visit(this);
		byteCode.emit(opCode);
	}

	//if (opCode == opcode::OP_CALL)
	//	node->expr->visit(this);

	//byteCode.emit(opcode::OP_CALL);

	if (node->discardReturnValue)
		byteCode.emit(opcode::OP_INDEX_DEC);
}

void GS2Compiler::Visit(StatementReturnNode *node)
{
	// pretty sure it goes on top
	if (node->expr)
		node->expr->visit(this);

	//byteCode.emit(opcode::OP_CONV_TO_STRING);
	byteCode.emit(opcode::OP_RET);
}

void GS2Compiler::Visit(StatementNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(StatementIfNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(StatementNewNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(StatementBreakNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(StatementContinueNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(StatementForNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(StatementForEachNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(StatementSwitchNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(StatementWhileNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(StatementWithNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(ExpressionNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(ExpressionUnaryOpNode *node) { Visit((Node *)node); }
void GS2Compiler::Visit(ExpressionListNode *node) { Visit((Node *)node); }