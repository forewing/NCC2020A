%{
    #include <stdio.h>
    #include "lex.yy.c"

    void yyerror(const char *s);
%}

%union {
    int type_int;
    float type_float;
}

%token <type_int> INT               // Int
%token <type_float> FLOAT           // Float
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

Program : ExtDefList
    ;

ExtDefList :
    | ExtDef ExtDefList
    ;

ExtDef : Specifier ExtDefList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    ;

ExtDefList : VarDec
    | VarDec COMMA ExtDefList
    ;


Specifier : TYPE
    | StructSpecifier
    ;

StructSpecifier : STRUCT OptTag LC DefList RC
    | STRUCT Tag
    ;

OptTag :
    | ID
    ;

Tag : ID
    ;

VarDec : ID
    | VarDec LB INT RB
    ;

FunDec : ID LP VarList RP
    | ID LP RP
    ;

VarList : ParamDec COMMA VarList
    | ParamDec
    ;

ParamDec : Specifier VarDec
    ;

CompSt : LC DefList StmtList RC
    ;

StmtList :
    | Stmt StmtList
    ;

Stmt : Exp SEMI
    | CompSt
    | RETURN Exp SEMI
    | IF LP Exp RP Stmt
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt
    ;

DefList :
    | Def DefList
    ;

Def : Specifier DecList SEMI
    ;

DecList : Dec
    | Dec COMMA DecList
    ;

Dec : VarDec
    | VarDec ASSIGNOP Exp
    ;

Exp : Exp ASSIGNOP Exp
    | Exp AND Exp
    | Exp OR Exp
    | Exp RELOP Exp
    | Exp PLUS Exp
    | Exp MINUS Exp
    | Exp STAR Exp
    | Exp DIV Exp
    | LP Exp RP
    | MINUS Exp
    | NOT Exp
    | ID LP Args RP
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID
    | ID
    | INT
    | FLOAT
    ;

Args : Exp COMMA Args
    | Exp
    ;

%%

void yyerror(const char* msg){
    fprintf(stderr, "Error type B at Line %d: %s.\n", yylineno, msg);
}