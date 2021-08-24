#pragma once

#ifndef GS2VISITOR_H
#define GS2VISITOR_H

#include <cstdarg>
#include "ast/ast.h"

inline std::string getArgList(std::vector<ExpressionNode *> & args)
{
    std::string argList;
    for (const auto& s : args)
    {
        argList += s->toString();
        argList += ",";
    }

    if (!argList.empty())
        argList.pop_back();

    return argList;
}

class GS2SourceVisitor : public NodeVisitor
{
public:
    int tabc;
    GS2SourceVisitor() : tabc(0) { }

    void printSpaces()
    {
        for (int i = 0; i < tabc; i++)
            printf("    ");
    }

    void print(const char *fmt, ...)
    {
        printSpaces();

        va_list argptr;
        va_start(argptr, fmt);
        vprintf(fmt, argptr);
        va_end(argptr);

        printf("\n");
    }

	virtual void Visit(Node *node)
    {
		print("Visit Node");
	}

    virtual void Visit(StatementNode *node)
    {
		print("Visit StatementNode");
	}

    virtual void Visit(StatementBlock *node)
    {
		// print("Visit StatementBlock");
        tabc++;
        for (const auto& n : node->statements)
        {
            if (n)
                n->visit(this);
        }
        tabc--;
	}

    virtual void Visit(StatementFnDeclNode *node)
    {
        tabc++;

        std::string argList;

        for (const auto& s : node->args)
        {
            argList += s->toString();
            argList += ",";
        }

        if (!argList.empty())
            argList.pop_back();

        std::string accessor;
        if (node->pub)
            accessor = "public ";

        print("%sfunction %s(%s) {", accessor.c_str(), node->ident.c_str(), argList.c_str());
        node->stmtBlock->visit(this);
        print("}");
        tabc--;
	}

    virtual void Visit(StatementIfNode *node)
    {
		// print("Visit StatementIfNode");

        tabc++;

        std::string exprStr;
        if (node->expr)
            exprStr = node->expr->toString();

        print("if (%s) {", exprStr.c_str());
        
        node->thenBlock->visit(this);
        if (node->elseBlock) {
            print("} else {");
            node->elseBlock->visit(this);
        }
        print("}");
        tabc--;
	}

    virtual void Visit(StatementNewNode *node)
    {
        tabc++;
        print("new %s(%s) {", node->ident.c_str(), getArgList(node->args).c_str());
        node->stmtBlock->visit(this);
        print("}");
        tabc--;
    }

    virtual void Visit(ExpressionNode *node)
    {
		print("Visit ExpressionNode");
	}

    virtual void Visit(ExpressionCastNode* node)
    {
        print("Visit ExpressionCastNode");
    }

    virtual void Visit(ExpressionIdentifierNode *node)
    {
		print("Visit ExpressionIdentifierNode");
	}

    virtual void Visit(ExpressionStringConstNode *node)
    {
		print("Visit ExpressionStringConstNode");
	}

    virtual void Visit(ExpressionIntegerNode *node)
    {
		print("Visit ExpressionIntegerNode");
	}

    virtual void Visit(ExpressionNumberNode *node)  {
		print("Visit ExpressionNumberNode");
	}

    virtual void Visit(ExpressionPostfixNode *node)
    {
        print("Visit ExpressionPostfixNode");
    }
    virtual void Visit(ExpressionTernaryOpNode* node)
    {
        tabc++;
        print("%s", node->toString().c_str());
        tabc--;
    }
    virtual void Visit(ExpressionBinaryOpNode *node)
    {
        tabc++;
        print("%s;", node->toString().c_str());
        tabc--;
    }

    virtual void Visit(ExpressionStrConcatNode *node)
    {
        tabc++;
        print("%s;", node->toString().c_str());
        tabc--;
    }

    virtual void Visit(ExpressionUnaryOpNode *node)
    {
        
    }

    virtual void Visit(ExpressionFnCallNode* node)
    {
        std::string argList;
        
        for (const auto& s : node->args)
        {
            argList += s->toString();
            argList += ",";
        }

        if (!argList.empty())
            argList.pop_back();
        
        tabc++;
        std::string funcCmd;
        if (node->objExpr != 0)
        {
            funcCmd.append(node->objExpr->toString());
            funcCmd.append(".");
        }
        funcCmd.append(node->funcExpr->toString());
        print("%s(%s);", funcCmd.c_str(), argList.c_str());
        tabc--;
    }

    virtual void Visit(ExpressionArrayIndexNode *node)
    {

    }

    virtual void Visit(ExpressionNewArrayNode *node)
    {

    }

    virtual void Visit(ExpressionNewObjectNode *node)
    {

    }

    virtual void Visit(ExpressionInOpNode *node)
    {

    }

    virtual void Visit(ExpressionListNode *node) {
        tabc++;
        print("%s;", node->toString().c_str());
        tabc--;
    }

    virtual void Visit(StatementForNode *node) {
        tabc++;

        std::string forStmt("for (");
        if (node->init)
            forStmt.append(node->init->toString());
        forStmt.append(";");

        if (node->cond)
            forStmt.append(node->cond->toString());
        forStmt.append(";");

        if (node->postop)
            forStmt.append(node->postop->toString());
        forStmt.append(") {");

        print("%s", forStmt.c_str());
        node->block->visit(this);
        print("}");
        tabc--;
    }

    virtual void Visit(StatementForEachNode *node) {
        tabc++;
        print("for (%s : %s) {", node->name->toString().c_str(), node->expr->toString().c_str());
        node->block->visit(this);
        print("}");
        tabc--;
    }

    virtual void Visit(StatementWhileNode *node) {
        tabc++;
        print("while (%s) {", node->expr->toString().c_str());
        node->block->visit(this);
        print("}");
        tabc--;
    }

    virtual void Visit(StatementWithNode *node) {
        tabc++;
        print("with (%s) {", node->expr->toString().c_str());
        node->block->visit(this);
        print("}");
        tabc--;
    }

    virtual void Visit(StatementSwitchNode *node) {
        print("switch (%s) {", node->expr->toString().c_str());
        tabc++;

        for (const auto& s : node->cases)
        {
            for (const auto& sn : s.exprList)
            {
                if (sn)
                {
                    print("case %s:", sn->toString().c_str());
                }
                else
                    print("default:");
            }

            print("{");
            s.block->visit(this);
            print("}");
        }

        tabc--;
        print("}");
    }

    virtual void Visit(StatementBreakNode *node)  {
        tabc++;
        print("break;");
        tabc--;
	}

    virtual void Visit(StatementContinueNode *node)  {
        tabc++;
        print("continue;");
        tabc--;
	}

    virtual void Visit(StatementReturnNode *node)  {
        tabc++;

        if (!node->expr)
            print("return;");
        else
        {
            print("return %s;", node->expr->toString().c_str());
        }

        tabc--;
	}

    virtual void Visit(ExpressionFnObject *node) {

    }
};

#endif
