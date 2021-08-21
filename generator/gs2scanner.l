%option nounput noyywrap 8bit  
%option reentrant bison-bridge bison-locations
%option extra-type="class ParserContext *"

%top{
#include <cstdint>
}

%{
#include "Parser.h"
#include "ast/ast.h"

#define YY_DECL int yylex(YYSTYPE * yylval_param, YYLTYPE * yylloc_param, YY_EXTRA_TYPE parser, yyscan_t yyscanner)

#include "gs2parser.tab.hh"

#define YY_USER_ACTION \
    yyextra->columnNumber += yyleng;

%}

DIGIT		[0-9]
ALPHA		[a-zA-Z_$]
ALPHANUM	[a-zA-Z_0-9]
string_const (\"(\\.|[^"])*\")
HEXNUM		[a-fA-F0-9]
%x C_COMMENT

%%

[ \t]	{ yyextra->columnNumber++; }; // ignore all whitespace
\n		{ yyextra->lineNumber++; yyextra->columnNumber = 0; };
"."		{return '.';}
","		{return ',';}
":"		{return ':';}
";"		{return ';';}
"|"		{return '|';}

"("		{return '(';}
")"		{return ')';}
"{"		{return '{';}
"}"		{return '}';}
"["		{return '[';}
"]"		{return ']';}

"?"		{return T_OPTERNARY;}
"!"		{return '!';}
"&&"	{return T_OPAND;}
"||"	{return T_OPOR;}
"=="	{return T_OPEQUALS;}
"!="	{return T_OPNOTEQUALS;}
"<"		{return '<';}
"<="	{return T_OPLESSTHANEQUAL;}
"=<"	{return T_OPLESSTHANEQUAL;}
">"		{return '>';}
">="	{return T_OPGREATERTHANEQUAL;}
"=>"	{return T_OPGREATERTHANEQUAL;}

"+"		{return '+';}
"-"		{return '-';}
"*"		{return '*';}
"/"		{return '/';}
"^"		{return '^';}
"%"		{return '%';}
"="		{return '=';}
":="	{return '=';}

"+="	{return T_OPADDASSIGN;}
"-="	{return T_OPSUBASSIGN;}
"*="	{return T_OPMULASSIGN;}
"/="	{return T_OPDIVASSIGN;}
"^="	{return T_OPPOWASSIGN;}
"%="	{return T_OPMODASSIGN;}
"@="	{return T_OPCATASSIGN;}

"--"	{return T_OPDECREMENT;}
"++"	{return T_OPINCREMENT;}

"&"		{return '&';}
"<<"	{return T_BITWISE_SHIFT_LEFT;}
">>"	{return T_BITWISE_SHIFT_RIGHT;}
"~"		{return T_BITWISE_INVERT;}
"xor"	{return T_BITWISE_XOR;}

"|="	{return T_BITWISE_OR_ASSIGN;}
"&="	{return T_BITWISE_AND_ASSIGN;}
"<<="	{return T_BITWISE_SHIFT_LEFT_ASSIGN;}
">>="	{return T_BITWISE_SHIFT_RIGHT_ASSIGN;}

	/*
		Reserved Keywords
	*/
"public"	{return T_KWPUBLIC;}
"if"		{return T_KWIF;}
"else"		{return T_KWELSE;}
	/*"elseif"	{return T_KWELSEIF;} */
"for"		{return T_KWFOR;}
"while"		{return T_KWWHILE;}
"break"		{return T_KWBREAK;}
"continue"	{return T_KWCONTINUE;}
"return"	{return T_KWRETURN;}
"function"	{return T_KWFUNCTION;}
"new"		{return T_KWNEW;}
"with"		{return T_KWWITH;}
"switch"	{return T_KWSWITCH;}
"case"		{return T_KWCASE;}
"default"	{return T_KWDEFAULT;}
"const"		{return T_KWCONST;}
"enum"		{return T_KWENUM;}
"int"		{return T_KWCAST_INT;}
"float"		{return T_KWCAST_FLOAT;}
"in"		{return T_KWIN;}
"@"			{yylval->cval = 0; return '@';}
"NL"		{yylval->cval = '\n'; return '@';}
"SPC"		{yylval->cval = ' '; return '@';}
"TAB"		{yylval->cval = '\t'; return '@';}

	/*
		Comments
	*/
"//".*"\n"			{ yyextra->lineNumber++;yyextra->columnNumber = 0; }

"/*"				{ BEGIN(C_COMMENT); }
<C_COMMENT>"*/"		{ BEGIN(INITIAL); }
<C_COMMENT>\n		{ yyextra->lineNumber++;yyextra->columnNumber = 0; }
<C_COMMENT>.		{ }

0x{HEXNUM}+			{yylval->ival = std::stoul(yytext, nullptr, 16); return T_INT;}
{DIGIT}*\.{DIGIT}+	{yylval->sval = yyextra->saveString(yytext, yyleng); return T_FLOAT;}
{DIGIT}+			{yylval->ival = atoi(yytext); return T_INT;}
{ALPHA}{ALPHANUM}*	{yylval->sval = yyextra->saveString(yytext, yyleng); return T_IDENTIFIER;}
{ALPHA}{ALPHANUM}*(::{ALPHA}{ALPHANUM}*)+ {yylval->sval = yyextra->saveString(yytext, yyleng); return T_IDENTIFIER;}
{string_const}		{yylval->sval = yyextra->saveString(yytext+1, yyleng - 2, true); return T_STRCONSTANT;}
%%

		/* TODO(joey): fix string escapes */
