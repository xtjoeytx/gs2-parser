%option noyywrap

%{
#include <stdio.h>
#include <string>
#include <set>

#include "ast.h"

#define YY_DECL int yylex()

#include "calc.tab.hh"

int g_LineNumber = 0;
std::set<std::string> stable;
const char * saveString(const char *s);

void setLexBuffer(const char *s);

/*
	{DIGIT}+"."{DIGIT}*     {yylval.fval = atof(yytext); return T_FLOAT;}
	[0-9]+\.[0-9]+  {yylval.fval = atof(yytext); return T_FLOAT;}
	[0-9]+          {yylval.ival = atoi(yytext); return T_INT;}
*/

%}

DIGIT		[0-9]
ALPHA		[a-zA-Z_]
ALPHANUM	[a-zA-Z_0-9]
string_const (\"(\\.|[^"])*\")

%%

[ \t]	; // ignore all whitespace
\n		{g_LineNumber++;};
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
"!"		{return T_OPNOT;}
"&&"	{return T_OPAND;}
"||"	{return T_OPOR;}
"=="	{return T_OPEQUALS;}
"!="	{return T_OPNOTEQUALS;}
"<"		{return T_OPLESSTHAN;}
"<="	{return T_OPLESSTHANEQUAL;}
">"		{return T_OPGREATERTHAN;}
">="	{return T_OPGREATERTHANEQUAL;}

"+"		{return T_OPADD;}
"-"		{return T_OPSUB;}
"*"		{return T_OPMUL;}
"/"		{return T_OPDIVIDE;}
"^"		{return T_OPPOW;}
"%"		{return T_OPMOD;}

"="		{return T_OPASSIGN;}
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

"null"		{yylval.ival = 0; return T_INT;}
"false"		{yylval.ival = 0; return T_INT;}
"true"		{yylval.ival = 1; return T_INT;}
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

	/*
		Comments
	*/
"//".*"\n" {
	g_LineNumber++;
}

"/*" {
		int c = 0, l;
		for ( ; ; )
		{
		l = c;
		c = yyinput();

		// Is this an open comment?
		if ( c == EOF )
		{
			printf( "unexpected end of file found in comment" );
			break;
		}

		// Increment line numbers.
		else if ( c == '\n' )
			g_LineNumber++;

		// Did we find the end of the comment?
		else if ( l == '*' && c == '/' )
			break;
		}
	}

{DIGIT}+\.{DIGIT}+	{yylval.fval = atof(yytext); return T_FLOAT;}
{DIGIT}+			{yylval.ival = atoi(yytext); return T_INT;}
{ALPHANUM}+			{yytext[yyleng] = 0; yylval.sval = saveString(yytext); return T_IDENTIFIER;}
{string_const}		{yytext[yyleng] = 0; yylval.sval = saveString(yytext); printf("Constant: %s\n", yylval.sval); return T_STRCONSTANT;}
%%

void setLexBuffer(const char *s) {
	yy_switch_to_buffer(yy_scan_string(s));
}

const char * saveString(const char *s) {
	auto ins = stable.insert(s);
	return ins.first->c_str();
}