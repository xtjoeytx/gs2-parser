#include "GS2Compiler.h"
#include "ast.h"

void GS2Compiler::Visit(Node *node)
{
	fprintf(stderr, "Unimplemented node type: %s\n", node->NodeType());
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
		byteCode.emit(char(0xF4));
		byteCode.emit(short(0));

		byteCode.emit(opcode::OP_RET);
	}

	// this code would jump over the function definition block,
	// but it seems graal jumps to the last possible opcode+1
	// byteCode.emit(short(byteCode.getBytecodePos()), jumpIdx);
}

void GS2Compiler::Visit(ExpressionBinaryOpNode *node)
{
	Visit((Node*)node);
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
	}

	node->expr->visit(this);

	byteCode.emit(opcode::OP_CALL);
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