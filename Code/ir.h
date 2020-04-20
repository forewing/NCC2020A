#ifndef IR_H
#define IR_H

#include <stdlib.h>

#include "helper.h"

typedef struct IrOprand {
    enum {
        OP_NULL,
        OP_CONST,
        OP_VAR,
        OP_GETADDR,
        OP_GETDATA,
        OP_FUNC,
        OP_LABEL,
        OP_TEMP
    } type;
    union {
        const char* data_str;
        int data_int;
    };
} IrOprand;

typedef struct IrCode {
    struct IrCode* prev;
    struct IrCode* next;

    enum {
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
    } type;

    IrOprand* x;
    IrOprand* y;
    IrOprand* z;
} IrCode;

IrOprand* IrOprand_new();
IrCode* IrCode_new();
void IrCode_insert(IrCode* pos, IrCode* elem);
void IrCode_delete(IrCode* pos);

extern IrCode* ircode_list;
extern int tmpvar_num;
extern int label_num;

#endif