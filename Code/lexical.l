%{
    #include <stdio.h>
    #include "syntax.tab.h"
    #include "helper.h"

    #define LEX_REGISTE(__LEX_TYPE__) { \
        LOG("(%d)\t" STRING_OF(__LEX_TYPE__) "\t%s\n", yylineno, yytext); \
        return(__LEX_TYPE__); \
    }
%}

%option yylineno

delim   [ \t\n\r]
ws      {delim}+
dec     0|([1-9][0-9]*)
letter_ [A-Za-z_]
digit   [0-9]
id      {letter_}({letter_}|{digit})*

%%

    /* Ignore */
{ws}    {}

    /* INT */
{dec}   {LEX_REGISTE(INT)}
0[0-7]+    {LEX_REGISTE(INT)}
0(x|X)[0-9a-fA-F]+   {LEX_REGISTE(INT)}

    /* FLOAT */
{dec}\.[0-9]+   {LEX_REGISTE(FLOAT)}
(([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))[eE][+-]?[0-9]+  {LEX_REGISTE(FLOAT)}

    /* Keywords */
"struct"    {LEX_REGISTE(STRUCT)}
"return"    {LEX_REGISTE(RETURN)}
"if"        {LEX_REGISTE(IF)}
"else"      {LEX_REGISTE(ELSE)}

    /* Types */
"int"   {LEX_REGISTE(TYPE)}
"float" {LEX_REGISTE(TYPE)}

    /* Marks */
{id}    {LEX_REGISTE(ID)}
";"     {LEX_REGISTE(SEMI)}
","     {LEX_REGISTE(COMMA)}
"."     {LEX_REGISTE(DOT)}
"="     {LEX_REGISTE(ASSIGNOP)}
"("     {LEX_REGISTE(LP)}
")"     {LEX_REGISTE(RP)}
"["     {LEX_REGISTE(LB)}
"]"     {LEX_REGISTE(RB)}
"{"     {LEX_REGISTE(LC)}
"}"     {LEX_REGISTE(RC)}

    /* RELOPs */
">"     {LEX_REGISTE(RELOP)}
"<"     {LEX_REGISTE(RELOP)}
">="    {LEX_REGISTE(RELOP)}
"<="    {LEX_REGISTE(RELOP)}
"=="    {LEX_REGISTE(RELOP)}
"!="    {LEX_REGISTE(RELOP)}

    /* ArithOPs */
"+"     {LEX_REGISTE(PLUS)}
"-"     {LEX_REGISTE(MINUS)}
"*"     {LEX_REGISTE(STAR)}
"/"     {LEX_REGISTE(DIV)}

    /* LogicOPs */
"&&"    {LEX_REGISTE(AND)}
"||"    {LEX_REGISTE(OR)}
"!"     {LEX_REGISTE(NOT)}

    /* Invalid */

.       {printf("Error type A at Line %d: Mysterious character \"%s\".\n", yylineno, yytext);}

%%
