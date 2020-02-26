%code {
    #include <stdio.h>
    #include "lex.yy.c"
    #include "helper.h"

    // #define YYDEBUG 1
    // int yydebug = 1;

    void yyerror(const char *s);

    #define syn_error(__msg__) { \
        fprintf(stderr, "Error type B at Line %d: %s.\n", yylineno, __msg__); \
        bug_add; \
    }

    #define SYN_REGISTE(__SYN_TYPE__, __SYN_PARENT__, __SYN_PARENT_LOC__, ...) { \
        LOG_SYN("(%d)-%d\t" STRING_OF(__SYN_TYPE__) "\n", __SYN_PARENT_LOC__.first_line, HELPER_NARG(__VA_ARGS__)); \
        __SYN_PARENT__ = tree_new(STRING_OF(__SYN_TYPE__), HELPER_NARG(__VA_ARGS__)); \
        __SYN_PARENT__->node_type = NODE_NOTERM; \
        __SYN_PARENT__->lineno = __SYN_PARENT_LOC__.first_line; \
        tree_set_children(__SYN_PARENT__, __VA_ARGS__); \
    }

    #define SYN_REGISTE_EMPTY(__SYN_TYPE__, __SYN_PARENT__) { \
        __SYN_PARENT__ = tree_new(STRING_OF(__SYN_TYPE__), 0); \
        __SYN_PARENT__->node_type = NODE_EMPTY; \
    }

    TreeNode* tree_root;

}

%code requires {
    #include "tree.h"
    #define YYSTYPE TreeNode*

    #define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
}

%token INT                          // Int
%token FLOAT                        // Float
%token ID                           // ID
%token SEMI COMMA DOT               // ; , .
%token ASSIGNOP RELOP               // =, > | < | >= | <= | == |!=
%token PLUS MINUS STAR DIV          // +, -, *, /
%token AND OR NOT                   // &&, ||, !
%token TYPE                         // int | float
%token LP RP LB RB LC RC            // (, ), [, ], {, }
%token STRUCT RETURN IF ELSE WHILE  // Keywords



%%

Program : ExtDefList    {SYN_REGISTE(Program, $$, @$, $1); tree_root = $$;}
    ;

ExtDefList : /* empty */    {SYN_REGISTE_EMPTY(ExtDefList, $$)}
    | ExtDef ExtDefList     {SYN_REGISTE(ExtDefList, $$, @$, $1, $2)}
    ;

ExtDef : Specifier ExtDefList SEMI  {SYN_REGISTE(ExtDef, $$, @$, $1, $2, $3)}
    | Specifier SEMI                {SYN_REGISTE(ExtDef, $$, @$, $1, $2)}
    | Specifier FunDec CompSt       {SYN_REGISTE(ExtDef, $$, @$, $1, $2, $3)}
    | Specifier error CompSt    /* ERROR! */
    | Specifier error SEMI      /* ERROR! */
    | error ExtDefList SEMI     /* ERROR! */
    | error SEMI                /* ERROR! */
    ;

ExtDefList : VarDec             {SYN_REGISTE(ExtDefList, $$, @$, $1)}
    | VarDec COMMA ExtDefList   {SYN_REGISTE(ExtDefList, $$, @$, $1, $2, $3)}
    ;


Specifier : TYPE        {SYN_REGISTE(Specifier, $$, @$, $1)}
    | StructSpecifier   {SYN_REGISTE(Specifier, $$, @$, $1)}
    ;

StructSpecifier : STRUCT OptTag LC DefList RC   {SYN_REGISTE(StructSpecifier, $$, @$, $1, $2, $3, $4, $5)}
    | STRUCT Tag    {SYN_REGISTE(StructSpecifier, $$, @$, $1, $2)}
    | STRUCT error LC DefList RC    /* ERROR! */
    | STRUCT OptTag LC error RC     /* ERROR! */
    ;

OptTag : /* empty */    {SYN_REGISTE_EMPTY(OptTag, $$)}
    | ID    {SYN_REGISTE(OptTag, $$, @$, $1)}
    ;

Tag : ID    {SYN_REGISTE(Tag, $$, @$, $1)}
    ;

VarDec : ID             {SYN_REGISTE(VarDec, $$, @$, $1)}
    | VarDec LB INT RB  {SYN_REGISTE(VarDec, $$, @$, $1, $2, $3, $4)}
    | error LB INT RB       /* ERROR! */
    | VarDec LB error       /* ERROR! */
    ;

FunDec : ID LP VarList RP   {SYN_REGISTE(FunDec, $$, @$, $1, $2, $3, $4)}
    | ID LP RP              {SYN_REGISTE(FunDec, $$, @$, $1, $2, $3)}
    | error LP RP           /* ERROR! */
    | ID error              /* ERROR! */
    | error LP VarList RP   /* ERROR! */
    ;

VarList : ParamDec COMMA VarList    {SYN_REGISTE(VarList, $$, @$, $1, $2, $3)}
    | ParamDec                      {SYN_REGISTE(VarList, $$, @$, $1)}
    ;

ParamDec : Specifier VarDec {SYN_REGISTE(ParamDec, $$, @$, $1, $2)}
    ;

CompSt : LC DefList StmtList RC {SYN_REGISTE(CompSt, $$, @$, $1, $2, $3, $4)}
    | LC error RC   /* ERROR! */
    ;

StmtList : /* empty */  {SYN_REGISTE_EMPTY(StmtList, $$)}
    | Stmt StmtList     {SYN_REGISTE(StmtList, $$, @$, $1, $2)}
    ;

Stmt : Exp SEMI                     {SYN_REGISTE(Stmt, $$, @$, $1, $2)}
    | CompSt                        {SYN_REGISTE(Stmt, $$, @$, $1)}
    | RETURN Exp SEMI               {SYN_REGISTE(Stmt, $$, @$, $1, $2, $3)}
    | IF LP Exp RP Stmt             {SYN_REGISTE(Stmt, $$, @$, $1, $2, $3, $4, $5)}
    | IF LP Exp RP Stmt ELSE Stmt   {SYN_REGISTE(Stmt, $$, @$, $1, $2, $3, $4, $5, $6, $7)}
    | WHILE LP Exp RP Stmt          {SYN_REGISTE(Stmt, $$, @$, $1, $2, $3, $4, $5)}
    | error SEMI            /* ERROR! */
    | RETURN error          /* ERROR! */
    | IF error Stmt         /* ERROR! */
    | IF LP Exp RP error    /* ERROR! */
    | IF LP Exp RP Stmt ELSE error  /* ERROR! */
    | WHILE error Stmt      /* ERROR! */
    | WHILE LP Exp RP error /* ERROR! */
    ;

DefList : /* empty */   {SYN_REGISTE_EMPTY(DefList, $$)}
    | Def DefList       {SYN_REGISTE(DefList, $$, @$, $1, $2)}
    ;

Def : Specifier DecList SEMI    {SYN_REGISTE(Def, $$, @$, $1, $2, $3)}
    | error DecList SEMI        /* ERROR! */
    | Specifier error SEMI      /* ERROR! */
    ;

DecList : Dec           {SYN_REGISTE(DecList, $$, @$, $1)}
    | Dec COMMA DecList {SYN_REGISTE(DecList, $$, @$, $1, $2, $3)}
    ;

Dec : VarDec                {SYN_REGISTE(Dec, $$, @$, $1)}
    | VarDec ASSIGNOP Exp   {SYN_REGISTE(Dec, $$, @$, $1, $2, $3)}
    ;

Exp : Exp ASSIGNOP Exp  {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | Exp AND Exp       {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | Exp OR Exp        {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | Exp RELOP Exp     {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | Exp PLUS Exp      {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | Exp MINUS Exp     {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | Exp STAR Exp      {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | Exp DIV Exp       {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | LP Exp RP         {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | MINUS Exp         {SYN_REGISTE(Exp, $$, @$, $1, $2)}
    | NOT Exp           {SYN_REGISTE(Exp, $$, @$, $1, $2)}
    | ID LP Args RP     {SYN_REGISTE(Exp, $$, @$, $1, $2, $3, $4)}
    | ID LP RP          {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | Exp LB Exp RB     {SYN_REGISTE(Exp, $$, @$, $1, $2, $3, $4)}
    | Exp DOT ID        {SYN_REGISTE(Exp, $$, @$, $1, $2, $3)}
    | ID                {SYN_REGISTE(Exp, $$, @$, $1)}
    | INT               {SYN_REGISTE(Exp, $$, @$, $1)}
    | FLOAT             {SYN_REGISTE(Exp, $$, @$, $1)}
    | LP error          /* ERROR! */
    | ID LP error       /* ERROR! */
    | error LP Args RP  /* ERROR! */
    | error LP RP       /* ERROR! */
    | error LB Exp RB   /* ERROR! */
    | Exp LB error      /* ERROR! */
    ;

Args : Exp COMMA Args   {SYN_REGISTE(Args, $$, @$, $1, $2, $3)}
    | Exp               {SYN_REGISTE(Args, $$, @$, $1)}
    ;

%%

void yyerror(const char* msg){
    syn_error(msg);
}
