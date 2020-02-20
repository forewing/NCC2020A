%{
    #include <stdio.h>
    #include "lex.yy.c"

    void yyerror(const char *s);
%}

%token INT FLOAT                    // Values
%token ID                           // ID
%token SEMI COMMA                   // ; ,
%token ASSIGNOP RELOP               // =, > | < | >= | <= | == |!=
%token PLUS MINUS STAR DIV          // +, -, *, /
%token AND OR NOT                   // &&, ||, !
%token DOT                          // .
%token TYPE                         // int | float
%token LP RP LB RB LC RC            // (, ), [, ], {, }
%token STRUCT RETURN IF ELSE WHILE  // Keywords

%%

Relop : PLUS
    ;

%%
