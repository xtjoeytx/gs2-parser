%{

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "ast.h"
#include "GS2SourceVisitor.h"
#include "GS2Compiler.h"

#include <fstream>
#include <streambuf>

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern void setLexBuffer(const char *s);

extern int g_LineNumber;
StatementBlock *stmtBlock = nullptr;

void yyerror(const char* s);
%}

%union {
	int ival;
	float fval;
	const char *sval;
	StatementNode *stmtNode;
	StatementBlock *stmtBlock;
	StatementIfNode *stmtIfNode;
	StatementFnDeclNode *fnStmtNode;
	StatementNewNode *stmtNewNode;
	StatementWhileNode *stmtWhileNode;
	StatementWithNode *stmtWithNode;
	ExpressionNode *exprNode;
	ExpressionIdentifierNode *exprIdentNode;
	ExpressionFnCallNode *exprCallNode;
	ExpressionObjectAccessNode *exprObjectAccessNode;
	ExpressionBinaryOpNode *exprBinaryNode;
	ExpressionUnaryOpNode *exprUnaryNode;
	ExpressionListNode *exprListNode;
	std::vector<ExpressionNode *> *argList;

	StatementSwitchNode *stmtSwitchNode;
	CaseNode *caseNode;
	std::vector<CaseNode *> *caseNodeList;
}

%token<ival> T_INT
%token<fval> T_FLOAT
%token<sval> T_IDENTIFIER
%token<sval> T_STRCONSTANT
%token '.' ',' ':' ';' '|' '@'
%token '(' ')'
%token '{' '}'
%token '[' ']'
%token T_TRUE T_FALSE

%token T_OPTERNARY
%token T_OPNOT
%token T_OPAND T_OPOR
%token T_OPEQUALS T_OPNOTEQUALS
%token T_OPLESSTHAN T_OPLESSTHANEQUAL
%token T_OPGREATERTHAN T_OPGREATERTHANEQUAL

%token '+' '-' '*' '/' '^' '%'
%token T_OPASSIGN
%token T_OPADDASSIGN T_OPSUBASSIGN T_OPMULASSIGN T_OPDIVASSIGN T_OPPOWASSIGN T_OPMODASSIGN
%token T_OPDECREMENT T_OPINCREMENT

%token T_KWPUBLIC
%token T_KWIF T_KWELSE T_KWELSEIF T_KWFOR T_KWWHILE T_KWBREAK T_KWCONTINUE T_KWRETURN
%token T_KWFUNCTION T_KWNEW T_KWWITH
%token T_KWSWITCH T_KWCASE T_KWDEFAULT
%token T_QUIT

%right T_OPASSIGN
%left '['
%left T_OPOR
%left T_OPAND
%left '^'
%left '@'
%left T_OPLESSTHAN T_OPLESSTHANEQUAL
%left T_OPGREATERTHAN T_OPGREATERTHANEQUAL
%left T_OPEQUALS T_OPNOTEQUALS
%left '+' '-'
%left '*' '/' '%'
%right T_OPNOT T_OPDECREMENT T_OPINCREMENT
%left '.'

%type<exprNode> constant expr primary_expression
%type<exprNode> expr_intconst
%type<exprIdentNode> expr_ident
%type<exprNode> expr_strconst
%type<exprUnaryNode> expr_ops_unary
%type<exprBinaryNode> expr_ops_binary expr_ops_comparison
%type<exprCallNode> expr_fncall
%type<exprListNode> expr_arraylist
%type<exprObjectAccessNode> expr_objaccess
%type<stmtIfNode> stmt_if
%type<stmtBlock> stmt_list
%type<stmtNode> stmt
%type<stmtNode> stmt_ret stmt_break stmt_continue stmt_expr
%type<stmtBlock> stmt_block
%type<stmtNewNode> stmt_new
%type<stmtNode> stmt_for
%type<stmtWhileNode> stmt_while
%type<stmtWithNode> stmt_with
%type<argList> args_list_decl args_list
%type<stmtNode> decl_list decl
%type<fnStmtNode> stmt_fndecl

%type<stmtSwitchNode> stmt_switch
%type<caseNode> stmt_caseblock
%type<caseNodeList> stmt_caseblock_list


%start program

%%

program:
	decl_list { stmtBlock = (StatementBlock *)$1; }
	;

decl_list: 					{ $$ = new StatementBlock(); }
	| decl_list decl 		{ ((StatementBlock *)$1)->append($2); }
	;

decl:
	stmt 					{ $$ = $1; }
	| stmt_fndecl 			{ $$ = $1; }
	;

stmt_list:
	stmt 					{ $$ = new StatementBlock(); $$->append($1); }
	| stmt_list stmt 		{ ((StatementBlock *)$1)->append($2); }
	;

stmt_block:
	'{' '}' 				{ $$ = new StatementBlock(); }
	| '{' stmt_list '}'		{ $$ = $2; }
	;

stmt: stmt_if				{ $$ = $1;}
	| stmt_ret
	| stmt_break
	| stmt_continue
	| stmt_expr
	| stmt_block 			{ $$ = $1; }
	| stmt_new 				{ $$ = $1; }
	| stmt_for				{ $$ = $1; }
	| stmt_while			{ $$ = $1; }
	| stmt_with				{ $$ = $1; }
	| stmt_switch 			{ $$ = $1; }
	;

stmt_new:
	T_KWNEW T_IDENTIFIER '(' args_list_decl ')' stmt_block	{ $$ = new StatementNewNode($2, $4, $6); }
	;

stmt_if:
	T_KWIF '(' expr ')' stmt 									{ $$ = new StatementIfNode($3, $5); }
	| T_KWIF '(' expr ')' stmt T_KWELSE stmt 				{ $$ = new StatementIfNode($3, $5, $7); }
	;

stmt_for:
	T_KWFOR '(' expr ';' expr ';' expr ')' stmt 		{ $$ = new StatementForNode($3, $5, $7, $9); }
	| T_KWFOR '(' expr ':' expr ')' stmt				{ $$ = new StatementForEachNode($3, $5, $7); }
	;

stmt_while:
	T_KWWHILE '(' expr ')' stmt 						{ $$ = new StatementWhileNode($3, $5); }
	;

stmt_with:
	T_KWWITH '(' expr ')' stmt							{ $$ = new StatementWithNode($3, $5); }
	;

stmt_break:
	T_KWBREAK ';' 										{ $$ = new StatementBreakNode(); }
	;

stmt_continue:
	T_KWCONTINUE ';'									{ $$ = new StatementContinueNode(); }
	;

stmt_ret:
	T_KWRETURN expr ';' 								{ $$ = new StatementReturnNode($2); }
	| T_KWRETURN ';' 									{ $$ = new StatementReturnNode(0); }
	;

stmt_switch:
	T_KWSWITCH '(' expr ')' '{' stmt_caseblock_list '}'	{ $$ = new StatementSwitchNode($3, $6); }
	;

stmt_caseblock_list:
	stmt_caseblock_list stmt_caseblock 					{ $1->push_back($2); }
	| stmt_caseblock 									{ $$ = new std::vector<CaseNode *>(); $$->push_back($1); }
	;

stmt_caseblock:
	T_KWCASE expr ':' stmt_list 						{ $$ = new CaseNode($2, $4); }
	| T_KWDEFAULT ':' stmt_list							{ $$ = new CaseNode(0, $3); }
	;

stmt_expr:
	';'													{ $$ = 0; }
	| expr ';'											{ $$ = $1; }
	;

stmt_fndecl:
	T_KWFUNCTION T_IDENTIFIER '(' args_list_decl ')' stmt_block						{ $$ = new StatementFnDeclNode($2, $4, $6); }
	| T_KWFUNCTION T_IDENTIFIER '.' T_IDENTIFIER '(' args_list_decl ')' stmt_block	{ $$ = new StatementFnDeclNode($4, $6, $8); }
	| T_KWPUBLIC stmt_fndecl { $$ = $2; $$->setPublic(true); }
	;

args_list_decl:
	{ $$ = 0; }
	| args_list { $$ = $1; }
	;

args_list:
	args_list ',' expr { $1->push_back($3); }
	| expr { $$ = new std::vector<ExpressionNode *>(); $$->push_back($1); }
	;

constant:
	expr_intconst
	| expr_strconst
	;

primary_expression:
	constant
	| expr_ident					{ $$ = $1; }
	| '(' expr ')'					{ $$ = $2; }
	;

// postfix_expression
// 	: primary_expression
// 	| postfix_expression '[' expr ']'
// 	| postfix_expression '.' expr_ident
// 	;

expr:
	primary_expression
	| expr_fncall 					{ $$ = $1; }
	| expr_objaccess 				{ $$ = $1; }
	| expr_arraylist				{ $$ = $1; }
	| expr_ops_binary 				{ $$ = $1; }
	| expr_ops_unary				{ $$ = $1; }
	| expr_ops_comparison			{ $$ = $1; }
	| expr '[' expr ']'				{ $$ = $1; }
	;

expr_ops_unary:
	'-' expr 						{ $$ = new ExpressionUnaryOpNode($2, "-"); }
	| T_OPNOT expr 					{ $$ = new ExpressionUnaryOpNode($2, "!"); }
	| T_OPDECREMENT expr 			{ $$ = new ExpressionUnaryOpNode($2, "--"); }
	| T_OPINCREMENT expr 			{ $$ = new ExpressionUnaryOpNode($2, "++"); }
	| expr T_OPINCREMENT			{ $$ = new ExpressionUnaryOpNode($1, "++"); }
	;

expr_ops_binary:
	expr '+' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, "+"); }
	| expr '-' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, "-"); }
	| expr '*' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, "*"); }
	| expr '/' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, "/"); }
	| expr '%' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, "%"); }
	| expr '^' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, "^"); }
	| expr T_OPASSIGN expr		 	{ $$ = new ExpressionBinaryOpNode($1, $3, "=", true); }
	| expr '@' expr		 			{ $$ = new ExpressionBinaryOpNode($1, $3, "@"); }
	;

expr_ops_comparison:
	expr T_OPEQUALS expr	 			{ $$ = new ExpressionBinaryOpNode($1, $3, "=="); }
	| expr T_OPNOTEQUALS expr	 		{ $$ = new ExpressionBinaryOpNode($1, $3, "!="); }
	| expr T_OPLESSTHAN expr	 		{ $$ = new ExpressionBinaryOpNode($1, $3, "<"); }
	| expr T_OPGREATERTHAN expr	 		{ $$ = new ExpressionBinaryOpNode($1, $3, ">"); }
	| expr T_OPLESSTHANEQUAL expr	 	{ $$ = new ExpressionBinaryOpNode($1, $3, "<="); }
	| expr T_OPGREATERTHANEQUAL expr	{ $$ = new ExpressionBinaryOpNode($1, $3, ">="); }
	| expr T_OPAND expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, "&&"); }
	| expr T_OPOR expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, "||"); }
	;


expr_intconst:
	T_INT							{ $$ = new ExpressionIntegerNode($1); }
	;

expr_ident:
	T_IDENTIFIER					{ $$ = new ExpressionIdentifierNode($1); }
	;

expr_strconst:
	T_STRCONSTANT 					{ $$ = new ExpressionStringConstNode($1); }
	;

expr_arraylist:
	'{' args_list '}' 				{ $$ = new ExpressionListNode($2); }
	;

expr_fncall:
	expr_ident '(' args_list_decl ')' 			{ $$ = new ExpressionFnCallNode($1, $3); }
	| expr_objaccess '(' args_list_decl ')' 	{ $$ = new ExpressionFnCallNode($1, $3); }
	;

expr_objaccess:
	expr '.' expr_ident							{ $$ = new ExpressionObjectAccessNode($1, $3); }
	;

%%

int main(int argc, const char *argv[]) {
#ifdef YYDEBUG
  yydebug = 1;
#endif
	// yyin = stdin;
	// do {
	// 	yyparse();
	// } while(!feof(yyin));

	std::string testStr;

	if (argc > 1)
	{
		std::ifstream t(argv[1]);
		std::string str((std::istreambuf_iterator<char>(t)),
						std::istreambuf_iterator<char>());
		
		testStr = std::move(str);
	}

	if (testStr.empty())
	{
		testStr = ""
			// "isstaff = 23;"
			"this.isstaff.lol();"
			"this.isstaff.lol2(abc);"
			"this.isstaff.lol3(abc,def,ghi);"
			"if (abcd) {"
			"	if (12345) {"
			"		return 3 + 2 * 6;"
			"	} else {"
			"		return 5 % 4;"
			"	}"
			"}"
			"if (hehehe) {"
			"	return hi;"
			"}"
			"function onCreated() {"
			"  if (lol) {"
			"    if (test)"
			"      return 0;"
			"	 else"
			"	   return 5;"
			"  }"
			"}"
			"public function onPlayerEnters(teee) {"
			"  if (lol) {"
			"    if (test)"
			"      return 0;"
			"  }"
			"}"
			"function onCreated() {"
			"  this.isstaff = \"hello_world\";"
			"  this.isstaff();"
			"  requesttext(\"options\", \"\");"
			"  sendtext(\"irc\", \"login\", player.nick);"
			"  onShowGUI();"
			"}";
	}

	// std::string testStr = "return 3 + 2 * 6;";
	setLexBuffer(testStr.c_str());
	yyparse();

	if (stmtBlock != nullptr)
	{
		TestNodeVisitor visit;
		printf("Children: %zu\n", stmtBlock->statements.size());
		visit.Visit(stmtBlock);

		GS2Compiler compilerVisitor;
		compilerVisitor.Visit(stmtBlock);

		auto byteCode = compilerVisitor.getByteCode();
		printf("Total length of bytecode w/ headers: %5zu\n", byteCode.length());

		auto buf = byteCode.buffer();

		FILE *file = fopen("weaponTestCode.dump", "wb");
		if (file)
		{
			uint8_t packetId = 140 + 32;
			fwrite(&packetId, sizeof(uint8_t), 1, file);
			fwrite(buf, sizeof(uint8_t), byteCode.length(), file);
			fclose(file);
		}
	}

	#ifdef _WIN32
	system("pause");
	#endif

	return 0;
}

void yyerror(const char* s) {
	fprintf(stderr, "Parse error (line %d): %s\n", g_LineNumber, s);
	exit(1);
}

