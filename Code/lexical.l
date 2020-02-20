%{
    #include <stdio.h>
    #include "syntax.tab.h"
%}

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
{dec}   {}
0[1-7][0-7]*    {}
0(x|X)[1-9a-fA-F][0-9a-fA-F]*   {}

    /* FLOAT */
{dec}\.[0-9]+   {}
(([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))[eE][+-]?[0-9]+  {}

    /* Keywords */
"struct"    {}
"return"    {}
"if"        {}
"else"      {}
"then"      {}

    /* Types */
"int"   {}
"float" {}

    /* Marks */
{id}    {}
";"     {}
","     {}
"."     {}
"="     {}
"("     {}
")"     {}
"["     {}
"]"     {}
"{"     {}
"}"     {}

    /* RELOPs */
">"     {}
"<"     {}
">="    {}
"<="    {}
"=="    {}
"!="    {}

    /* ArithOPs */
"+"     {}
"-"     {}
"*"     {}
"/"     {}

    /* LogicOPs */
"&&"    {}
"||"    {}
"!"     {}

%%
