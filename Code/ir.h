#ifndef IR_H
#define IR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "helper.h"
#include "tree.h"

enum OP_TYPE { OP_NULL, OP_CONST, OP_VAR, OP_GETADDR, OP_GETDATA, OP_FUNC, OP_LABEL, OP_TEMP };

typedef struct IrOprand {
    int type;
    union {
        const char* data_str;
        int data_int;
        struct IrOprand* data_op;
    };
} IrOprand;

enum CODE_TYPE {

    CODE_NOP,
    CODE_LABEL,
    CODE_FUNC,
    CODE_ASSIGN,
    CODE_ADD,
    CODE_SUB,
    CODE_MUL,
    CODE_DIV,
    CODE_GETADDR,
    CODE_GETDATA,
    CODE_SETDATA,
    CODE_GOTO,
    CODE_GOCOND,
    CODE_RET,
    CODE_DEC,
    CODE_ARG,
    CODE_CALL,
    CODE_PARAM,
    CODE_READ,
    CODE_WRITE
};

typedef struct IrCode {
    struct IrCode* prev;
    struct IrCode* next;

    IrOprand* x;
    IrOprand* y;
    IrOprand* z;

    int type;

    int data_int;

} IrCode;

IrOprand* IrOprand_new(int type);
IrOprand* IrOprand_new_int(int type, int data);
IrOprand* IrOprand_new_str(int type, const char* data);
IrOprand* IrOprand_new_op(int type, IrOprand* data);

#define OP_NEW_TEMP(__TMP_ID__) IrOprand_new_int(OP_TEMP, __TMP_ID__)
#define OP_NEW_CONST(__CONST_VAL__) IrOprand_new_int(OP_CONST, __CONST_VAL__)
#define OP_NEW_VAR(__VAR_NAME__) IrOprand_new_str(OP_VAR, __VAR_NAME__)
#define OP_NEW_LABEL(__LABEL_ID__) IrOprand_new_int(OP_LABEL, __LABEL_ID__)

IrCode* IrCode_new(int type, int data, IrOprand* x, IrOprand* y, IrOprand* z);

void IrCode_insert(IrCode* pos, IrCode* elem);
void IrCode_delete(IrCode* pos);

void IrCode_print(FILE* fp, IrCode* tail);
const char* IrOprand_print(IrOprand* op);

extern IrCode* ircode_list;
extern int tmpvar_num;
extern int label_num;
extern int compst_num;

int tmpvar_new();
int label_new();

void ircode_opt(IrCode* tail);
void ircode_opt_useless(IrCode* tail);
void ircode_opt_address(IrCode* tail);
void ircode_opt_eval(IrCode* tail);

#endif