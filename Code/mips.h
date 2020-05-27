#ifndef MIPS_H
#define MIPS_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "helper.h"
#include "ir.h"

#define MIPS_REG_NR 32

typedef enum mips_reg_type {
    MIPS_REG_ZERO = 0,
    MIPS_REG_AT = 1,
    MIPS_REG_V0 = 2,
    MIPS_REG_V1 = 3,
    MIPS_REG_A0 = 4,
    MIPS_REG_A1 = 5,
    MIPS_REG_A2 = 6,
    MIPS_REG_A3 = 7,
    MIPS_REG_T0 = 8,
    MIPS_REG_T1 = 9,
    MIPS_REG_T2 = 10,
    MIPS_REG_T3 = 11,
    MIPS_REG_T4 = 12,
    MIPS_REG_T5 = 13,
    MIPS_REG_T6 = 14,
    MIPS_REG_T7 = 15,
    MIPS_REG_S0 = 16,
    MIPS_REG_S1 = 17,
    MIPS_REG_S2 = 18,
    MIPS_REG_S3 = 19,
    MIPS_REG_S4 = 20,
    MIPS_REG_S5 = 21,
    MIPS_REG_S6 = 22,
    MIPS_REG_S7 = 23,
    MIPS_REG_T8 = 24,
    MIPS_REG_T9 = 25,
    MIPS_REG_K0 = 26,
    MIPS_REG_K1 = 27,
    MIPS_REG_GP = 28,
    MIPS_REG_SP = 29,
    MIPS_REG_FP = 30,
    MIPS_REG_RA = 31
} mips_reg_type;

typedef struct mips_var_t {
    IrOprand* op;
    int offset;
    struct mips_var_t *prev, *next;
} mips_var_t;

typedef struct mips_reg_t {
    int type;
    mips_var_t* var;
} mips_reg_t;

void mips_reg_init();
void mips_reg_load(int reg_id, IrOprand* op);

void mips_var_init();
mips_var_t* mips_var_new(IrOprand* op);
mips_var_t* mips_var_get(IrOprand* op);

void mips_print(FILE* fp, IrCode* code_list);

void mips_print_LABEL(IrCode* code);
void mips_print_FUNC(IrCode* code);
void mips_print_ASSIGN(IrCode* code);
void mips_print_ADD(IrCode* code);
void mips_print_SUB(IrCode* code);
void mips_print_MUL(IrCode* code);
void mips_print_DIV(IrCode* code);
void mips_print_GOTO(IrCode* code);
void mips_print_GOCOND(IrCode* code);
void mips_print_RET(IrCode* code);
void mips_print_DEC(IrCode* code);
void mips_print_ARG(IrCode* code);
void mips_print_CALL(IrCode* code);
void mips_print_PARAM(IrCode* code);
void mips_print_READ(IrCode* code);
void mips_print_WRITE(IrCode* code);

#endif