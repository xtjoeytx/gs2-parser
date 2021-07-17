%define api.pure full
%locations
%param { class ParserData *parser }
%param { yyscan_t scanner }

%code {
  int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, class ParserData *parser, yyscan_t scanner);
  void yyerror(YYLTYPE* yyllocp, class ParserData *parser, yyscan_t unused, const char* msg);
}

%{

#include <stdio.h>
#include "ast.h"

#include "Parser.h"

typedef void* yyscan_t;

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

	EnumList *enumList;
	EnumMember *enumMember;
}

%token<ival> T_INT
%token<sval> T_FLOAT
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
%token '='
%token T_OPADDASSIGN T_OPSUBASSIGN T_OPMULASSIGN T_OPDIVASSIGN T_OPPOWASSIGN T_OPMODASSIGN
%token T_OPDECREMENT T_OPINCREMENT

%token T_KWPUBLIC
%token T_KWIF T_KWELSE T_KWELSEIF T_KWFOR T_KWWHILE T_KWBREAK T_KWCONTINUE T_KWRETURN
%token T_KWFUNCTION T_KWNEW T_KWWITH T_KWENUM
%token T_KWSWITCH T_KWCASE T_KWDEFAULT
%token T_KWCAST_INT T_KWCAST_FLOAT

%right '='
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

%type<exprNode> expr
%type<exprNode> constant
%type<exprNode> expr_cast
%type<exprNode> expr_intconst expr_numberconst expr_strconst
%type<exprIdentNode> expr_ident
%type<exprUnaryNode> expr_ops_unary
%type<exprBinaryNode> expr_ops_binary expr_ops_comparison
%type<exprCallNode> expr_fncall
%type<exprNode> expr_new
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
%type<enumList> enum_list
%type<enumMember> enum_item

%start program

%%

program:
	decl_list				{ parser->prog = (StatementBlock *)$1; }
	;

decl_list: 					{ $$ = new StatementBlock(); }
	| decl_list decl 		{ ((StatementBlock *)$1)->append($2); }
	;

decl:
	stmt 					{ $$ = $1; }
	| stmt_fndecl 			{ $$ = $1; }
	| stmt_enum				{ $$ = 0; }
	;

stmt_enum:
	T_KWENUM '{' enum_list '}' ';'	{ parser->addEnum($3); }

enum_list:
	enum_item						{ $$ = new EnumList($1); }
	| enum_list ',' enum_item		{ $1->addMember($3); }
	;

enum_item:
	T_IDENTIFIER					{ $$ = new EnumMember($1); }
	| T_IDENTIFIER '=' T_INT		{ $$ = new EnumMember($1, $3); }
	| T_IDENTIFIER '=' '-' T_INT	{ $$ = new EnumMember($1, -$4); }
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
	/* | stmt_new 				{ $$ = $1; } */
	| stmt_for				{ $$ = $1; }
	| stmt_while			{ $$ = $1; }
	| stmt_with				{ $$ = $1; }
	| stmt_switch 			{ $$ = $1; }
	;
	
stmt_expr:
	';'						{ $$ = 0; }
	| expr ';'				{ $$ = $1; }
	;

	/*
		todo(joey): new should be an expression, not statement
	*/

stmt_new:
	T_KWNEW T_IDENTIFIER '(' args_list_decl ')' stmt_block	{ $$ = new StatementNewNode($2, $4, $6); }
	;

stmt_if:
	T_KWIF '(' expr ')' stmt 								{ $$ = new StatementIfNode($3, $5); }
	| T_KWIF '(' expr ')' stmt T_KWELSE stmt 				{ $$ = new StatementIfNode($3, $5, $7); }
	;

stmt_for:
	T_KWFOR '(' expr ';' expr ';' expr ')' stmt 			{ $$ = new StatementForNode($3, $5, $7, $9); }
	| T_KWFOR '(' ';' expr ';' expr ')' stmt 				{ $$ = new StatementForNode(0, $4, $6, $8); }
	| T_KWFOR '(' expr ':' expr ')' stmt					{ $$ = new StatementForEachNode($3, $5, $7); }
	;

stmt_while:
	T_KWWHILE '(' expr ')' stmt 							{ $$ = new StatementWhileNode($3, $5); }
	;

stmt_with:
	T_KWWITH '(' expr ')' stmt								{ $$ = new StatementWithNode($3, $5); }
	;

stmt_break:
	T_KWBREAK ';' 											{ $$ = new StatementBreakNode(); }
	;

stmt_continue:
	T_KWCONTINUE ';'										{ $$ = new StatementContinueNode(); }
	;

stmt_ret:
	T_KWRETURN expr ';' 									{ $$ = new StatementReturnNode($2); }
	| T_KWRETURN ';' 										{ $$ = new StatementReturnNode(0); }
	;

stmt_switch:
	T_KWSWITCH '(' expr ')' '{' stmt_caseblock_list '}'		{ $$ = new StatementSwitchNode($3, $6); }
	;

stmt_caseblock_list:
	stmt_caseblock_list stmt_caseblock 						{ $1->push_back($2); }
	| stmt_caseblock 										{ $$ = new std::vector<CaseNode *>(); $$->push_back($1); }
	;

stmt_caseblock:
	T_KWCASE expr ':' stmt_list 							{ $$ = new CaseNode($2, $4); }
	| T_KWDEFAULT ':' stmt_list								{ $$ = new CaseNode(0, $3); }
	;

stmt_fndecl:
	T_KWFUNCTION T_IDENTIFIER '(' args_list_decl ')' stmt_block						{ $$ = new StatementFnDeclNode($2, $4, $6); }
	| T_KWFUNCTION T_IDENTIFIER '.' T_IDENTIFIER '(' args_list_decl ')' stmt_block	{ $$ = new StatementFnDeclNode($4, $6, $8); }
	| T_KWPUBLIC stmt_fndecl														{ $$ = $2; $$->setPublic(true); }
	;

args_list_decl:
	{ $$ = 0; }
	| args_list						{ $$ = $1; }
	;

args_list:
	args_list ',' expr				{ $1->push_back($3); }
	| expr							{ $$ = new std::vector<ExpressionNode *>(); $$->push_back($1); }
	;

constant:
	expr_intconst
	| expr_numberconst
	| expr_strconst
	;

expr:
	constant						{ $$ = $1; }
	| expr_ident					{ $$ = $1; }
	| expr_cast						{ $$ = $1; }
	| expr_fncall 					{ $$ = $1; }
	| expr_new						{ $$ = $1; }
	| expr_objaccess 				{ $$ = $1; }
	| expr_arraylist				{ $$ = $1; }
	| expr_ops_binary 				{ $$ = $1; }
	| expr_ops_unary				{ $$ = $1; }
	| expr_ops_comparison			{ $$ = $1; }
	| expr '[' expr ']'				{ $$ = $1; }
	| '(' expr ')'					{ $$ = $2; }
	;

expr_ops_unary:
	'-' expr 						{ $$ = new ExpressionUnaryOpNode($2, ExpressionOp::UnaryMinus, true); }
	| T_OPNOT expr 					{ $$ = new ExpressionUnaryOpNode($2, ExpressionOp::UnaryNot, true); }
	| T_OPDECREMENT expr 			{ $$ = new ExpressionUnaryOpNode($2, ExpressionOp::Decrement, true); }
	| T_OPINCREMENT expr 			{ $$ = new ExpressionUnaryOpNode($2, ExpressionOp::Increment, true); }
	| expr T_OPINCREMENT			{ $$ = new ExpressionUnaryOpNode($1, ExpressionOp::Increment, false); }
	| expr T_OPDECREMENT			{ $$ = new ExpressionUnaryOpNode($1, ExpressionOp::Decrement, false); }
	;

expr_ops_binary:
	expr '+' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::Plus); }
	| expr '-' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::Minus); }
	| expr '*' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::Multiply); }
	| expr '/' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::Divide); }
	| expr '%' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::Mod); }
	| expr '^' expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::Pow); }
	| expr '=' expr		 			{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::Assign, true); }
	| expr '@' expr		 			{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::Concat); }
	;

expr_ops_comparison:
	expr T_OPEQUALS expr	 					{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::Equal); }
	| expr T_OPNOTEQUALS expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::NotEqual); }
	| expr T_OPLESSTHAN expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::LessThan); }
	| expr T_OPGREATERTHAN expr	 				{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::GreaterThan); }
	| expr T_OPLESSTHANEQUAL expr	 			{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::LessThanOrEqual); }
	| expr T_OPGREATERTHANEQUAL expr			{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::GreaterThanOrEqual); }
	| expr T_OPAND expr	 						{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::LogicalAnd); }
	| expr T_OPOR expr	 						{ $$ = new ExpressionBinaryOpNode($1, $3, ExpressionOp::LogicalOr); }
	;

expr_ident:
	T_IDENTIFIER								{ $$ = new ExpressionIdentifierNode($1); }
	;

expr_intconst:
	T_INT										{ $$ = new ExpressionIntegerNode($1); }
	;

expr_numberconst:
	T_FLOAT										{ $$ = new ExpressionNumberNode($1); }
	;

expr_strconst:
	T_STRCONSTANT 								{ $$ = new ExpressionStringConstNode($1); }
	;

expr_arraylist:
	'{' args_list '}' 							{ $$ = new ExpressionListNode($2); }
	;

expr_cast:
	T_KWCAST_INT '(' expr ')'					{ $$ = new ExpressionCastNode($3, ExpressionCastNode::CastType::INTEGER); }
	| T_KWCAST_FLOAT '(' expr ')'				{ $$ = new ExpressionCastNode($3, ExpressionCastNode::CastType::FLOAT); }
	;

expr_fncall:
	expr_ident '(' args_list_decl ')' 			{ $$ = new ExpressionFnCallNode($1, nullptr, $3); }
	| expr_objaccess '(' args_list_decl ')' 	{ $$ = new ExpressionFnCallNode($1->right, $1, $3); }
	;

expr_new:
	T_KWNEW expr_ident '(' args_list_decl ')'	{ $$ = new ExpressionNewNode($2, $4); }
	;

expr_objaccess:
	expr_objaccess '.' expr_ident				{ $1->nodes.push_back($1->right); $1->right = $3; }
	| expr_ident '.' expr_ident					{ $$ = new ExpressionObjectAccessNode($1, $3); }
	| expr_strconst '.' expr_ident				{ $$ = new ExpressionObjectAccessNode($1, $3); }
	;

%%

#include "lex.yy.h"

void yyerror(YYLTYPE* yyllocp, class ParserData *parser, yyscan_t unused, const char* s) {
	fprintf(stderr, "Parse error (line %d): %s\n", parser->lineNumber, s);
	
	#ifdef _WIN32
	system("pause");
	#endif

	exit(1);
}
