%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "syntax.tab.h"
    #include "helper.h"
    #include "tree.h"

    #define LEX_REGISTE_DATA(__LEX_TYPE__, __LEX_DATA_TYPE__, __LEX_DATA_VALUE__) \
    if (!in_block_comment) { \
        LOG("(%d)\t" STRING_OF(__LEX_TYPE__) "\t%s\n", yylineno, yytext); \
        yylval = tree_new(STRING_OF(__LEX_TYPE__), 0); \
        yylval->type = STATE_TERM; \
        yylval->lineno = yylineno; \
        yylval->__LEX_DATA_TYPE__ = __LEX_DATA_VALUE__; \
        return(__LEX_TYPE__); \
    }

    #define LEX_REGISTE(__LEX_TYPE__) \
        LEX_REGISTE_DATA(__LEX_TYPE__, data_int, 0)

    bool in_block_comment = false;
%}

%option yylineno

delim   [ \t\n\r]
ws      {delim}+
dec     0|([1-9][0-9]*)
letter_ [A-Za-z_]
digit   [0-9]
id      {letter_}({letter_}|{digit})*

%%

    /* Comment */
"/*"    {
    if (in_block_comment){
        syn_error("Nested block comment beginner");
    } else {
        in_block_comment = true;
        LOG("Starting block comment at Line %d\n", yylineno);
    }
}

"*/"    {
    if (in_block_comment){
        LOG("Terminating block comment at Line %d\n", yylineno);
        in_block_comment = false;
    } else {
        syn_error("Unexpected block comment terminator");
    }
}

\/\/.*$ {
    if (in_block_comment){
        REJECT;
    } else {
        LOG("Line comment at Line %d\n", yylineno);
    }
}

    /* Ignore */

{ws}    {}

    /* INT */
{dec}               {LEX_REGISTE_DATA(INT, data_int, strtol(yytext, NULL, 10))}
0[0-7]+             {LEX_REGISTE_DATA(INT, data_int, strtol(yytext, NULL, 8))}
0(x|X)[0-9a-fA-F]+  {LEX_REGISTE_DATA(INT, data_int, strtol(yytext, NULL, 16))}

    /* FLOAT */
({dec}\.[0-9]+)|((([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))[eE][+-]?[0-9]+) {LEX_REGISTE_DATA(FLOAT, data_float, atof(yytext))}

    /* Keywords */
"struct"    {LEX_REGISTE(STRUCT)}
"return"    {LEX_REGISTE(RETURN)}
"if"        {LEX_REGISTE(IF)}
"else"      {LEX_REGISTE(ELSE)}

    /* Types */
"int"   {LEX_REGISTE_DATA(TYPE, data_int, TYPE_INT)}
"float" {LEX_REGISTE_DATA(TYPE, data_int, TYPE_FLOAT)}

    /* Marks */
{id}    {LEX_REGISTE_DATA(ID, data_str, yytext)} // BUG
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
">"     {LEX_REGISTE_DATA(RELOP, data_int, RELOP_GT)}
"<"     {LEX_REGISTE_DATA(RELOP, data_int, RELOP_LT)}
">="    {LEX_REGISTE_DATA(RELOP, data_int, RELOP_GE)}
"<="    {LEX_REGISTE_DATA(RELOP, data_int, RELOP_LE)}
"=="    {LEX_REGISTE_DATA(RELOP, data_int, RELOP_EQ)}
"!="    {LEX_REGISTE_DATA(RELOP, data_int, RELOP_NE)}

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

.       {if (!in_block_comment) lex_error("Mysterious character");}

%%
