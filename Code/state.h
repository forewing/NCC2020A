#ifndef STATE_H
#define STATE_H

enum STATE {
    STATE_INT,
    STATE_FLOAT,
    STATE_ID,
    STATE_SEMI,
    STATE_COMMA,
    STATE_DOT,
    STATE_ASSIGNOP,
    STATE_RELOP,
    STATE_PLUS,
    STATE_MINUS,
    STATE_STAR,
    STATE_DIV,
    STATE_AND,
    STATE_OR,
    STATE_NOT,
    STATE_TYPE,
    STATE_LP,
    STATE_RP,
    STATE_LB,
    STATE_RB,
    STATE_LC,
    STATE_RC,
    STATE_STRUCT,
    STATE_RETURN,
    STATE_IF,
    STATE_ELSE,
    STATE_WHILE,
    STATE_Program,
    STATE_ExtDefList,
    STATE_ExtDef,
    STATE_ExtDecList,
    STATE_Specifier,
    STATE_StructSpecifier,
    STATE_OptTag,
    STATE_Tag,
    STATE_VarDec,
    STATE_FunDec,
    STATE_VarList,
    STATE_ParamDec,
    STATE_CompSt,
    STATE_StmtList,
    STATE_Stmt,
    STATE_DefList,
    STATE_Def,
    STATE_DecList,
    STATE_Dec,
    STATE_Exp,
    STATE_Args
};

#define TO_STATE(__ORIGIN_NAME__) STATE_##__ORIGIN_NAME__

void symtab_build();

#endif