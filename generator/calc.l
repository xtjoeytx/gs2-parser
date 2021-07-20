%option nounput noyywrap 8bit  
%option reentrant bison-bridge bison-locations
%option extra-type="class ParserData *"

%top{
#include <cstdint>
}

%{
#include "Parser.h"
#include "ast.h"

#define YY_DECL int yylex(YYSTYPE * yylval_param, YYLTYPE * yylloc_param, YY_EXTRA_TYPE parser, yyscan_t yyscanner)

#include "calc.tab.hh"

%}

DIGIT		[0-9]
ALPHA		[a-zA-Z_]
ALPHANUM	[a-zA-Z_0-9]
string_const (\"(\\.|[^"])*\")

%%

[ \t]	; // ignore all whitespace
\n		{yyextra->lineNumber++; 	printf("line: %d\n", yyextra->lineNumber);};
"."		{return '.';}
","		{return ',';}
":"		{return ':';}
";"		{return ';';}
"|"		{return '|';}
"@"		{return '@';}

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
"<"		{return T_OPLESSTHAN;}
"<="	{return T_OPLESSTHANEQUAL;}
">"		{return T_OPGREATERTHAN;}
">="	{return T_OPGREATERTHANEQUAL;}

"+"		{return '+';}
"-"		{return '-';}
"*"		{return '*';}
"/"		{return '/';}
"^"		{return '^';}
"%"		{return '%';}
"="		{return '=';}

"+="	{return T_OPADDASSIGN;}
"-="	{return T_OPSUBASSIGN;}
"*="	{return T_OPMULASSIGN;}
"/="	{return T_OPDIVASSIGN;}
"^="	{return T_OPPOWASSIGN;}
"%="	{return T_OPMODASSIGN;}

"--"	{return T_OPDECREMENT;}
"++"	{return T_OPINCREMENT;}

	/*
		Reserved Keywords
	*/

"null"		{yylval->ival = 0; return T_INT;}
"false"		{yylval->ival = 0; return T_INT;}
"true"		{yylval->ival = 1; return T_INT;}
"public"	{return T_KWPUBLIC;}
"if"		{return T_KWIF;}
"else"		{return T_KWELSE;}
"elseif"	{return T_KWELSEIF;}
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
"enum"		{return T_KWENUM;}
"int"		{return T_KWCAST_INT;}
"float"		{return T_KWCAST_FLOAT;}
"in"		{return T_KWIN;}

	/*
		Comments
	*/
"//".*"\n" {
	yyextra->lineNumber++;
}

"/*" {
		int c = 0, l;
		for ( ; ; )
		{
		l = c;
		c = yyinput(yyscanner);

		// Is this an open comment?
		if ( c == EOF )
		{
			printf( "unexpected end of file found in comment" );
			break;
		}

		// Increment line numbers.
		else if ( c == '\n' )
			yyextra->lineNumber++;

		// Did we find the end of the comment?
		else if ( l == '*' && c == '/' )
			break;
		}
	}
	
{DIGIT}*\.{DIGIT}+	{yylval->sval = yyextra->saveString(yytext, yyleng); return T_FLOAT;}
{DIGIT}+			{yylval->ival = atoi(yytext); return T_INT;}
{ALPHANUM}+			{yylval->sval = yyextra->saveString(yytext, yyleng); return T_IDENTIFIER;}
{string_const}		{yylval->sval = yyextra->saveString(yytext+1, yyleng - 2); return T_STRCONSTANT;}
%%
