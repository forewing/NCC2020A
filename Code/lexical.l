%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "syntax.tab.h"
    #include "helper.h"
    #include "tree.h"

    #define lex_error(__msg__, __text__) { \
        fprintf(stderr, "Error type A at Line %d: %s%s.\n", yylineno, __msg__, __text__); \
        bug_add; \
    }

    #define LEX_REGISTE_DATA(__LEX_TYPE__, __LEX_DATA_TYPE__, __LEX_DATA_VALUE__) { \
        LOG_LEX("(%d)\t" STRING_OF(__LEX_TYPE__) "\t%s\n", yylineno, yytext); \
        yylval = tree_new(STRING_OF(__LEX_TYPE__), 0); \
        yylval->node_type = NODE_TERM; \
        yylval->lex_type = __LEX_TYPE__; \
        yylval->lineno = yylineno; \
        yylval->__LEX_DATA_TYPE__ = __LEX_DATA_VALUE__; \
        return(__LEX_TYPE__); \
    }

    #define LEX_REGISTE(__LEX_TYPE__) \
        LEX_REGISTE_DATA(__LEX_TYPE__, data_int, 0)

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

"/*" {
    LOG_LEX("Block comment start at Line: %d.\n", yylineno);
    while(1) {
        int c;
        while ((c = input()) != '*' && c != 0);
        if (c == '*') {
            while ((c = input()) == '*');
            if (c == '/') {
                LOG_LEX("Block comment ends at Line: %d.\n", yylineno);
                break;
            }
        } else if (c == 0){
            lex_error("Unexcepted EOF in block comment, maybe you forget: ", "*/");
            break;
        }
    }
}

"*/" {lex_error("Unexcepted block comment terminator: ", yytext);}

"//" {
    int c;
    LOG_LEX("Line comment at Line %d\n", yylineno);
    while((c = input()) != '\n' && c != 0);
}

    /* Ignore */

{ws}    {}

    /* INT */
{dec}               {LEX_REGISTE_DATA(INT, data_int, strtol(yytext, NULL, 10))}
0[0-7]+             {LEX_REGISTE_DATA(INT, data_int, strtol(yytext, NULL, 8))}
0(x|X)[0-9a-fA-F]+  {LEX_REGISTE_DATA(INT, data_int, strtol(yytext, NULL, 16))}

    /* FLOAT */
([0-9]+\.[0-9]+)|((([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))[eE][+-]?[0-9]+) {LEX_REGISTE_DATA(FLOAT, data_float, atof(yytext))}

    /* Keywords */
"struct"    {LEX_REGISTE(STRUCT)}
"return"    {LEX_REGISTE(RETURN)}
"if"        {LEX_REGISTE(IF)}
"else"      {LEX_REGISTE(ELSE)}
"while"     {LEX_REGISTE(WHILE)}

    /* Types */
"int"   {LEX_REGISTE_DATA(TYPE, data_int, TYPE_INT)}
"float" {LEX_REGISTE_DATA(TYPE, data_int, TYPE_FLOAT)}

{id}    {
    char* t = malloc(strlen(yytext) + 1);
    strcpy(t, yytext);
    LEX_REGISTE_DATA(ID, data_str, t)
}

    /* Marks */
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

.       {lex_error("Mysterious character: ", yytext);}

%%
