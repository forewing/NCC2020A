#include "mips.h"

char* mips_reg_name[MIPS_REG_NR] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2",
                                    "$t3",   "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5",
                                    "$s6",   "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};

mips_reg_t mips_regs[MIPS_REG_NR];
mips_var_t* mips_var_list;

void mips_reg_init() {
    for (int i = 0; i < MIPS_REG_NR; i++) {
        mips_regs[i].type = i;
        mips_regs[i].var = NULL;
    }
}

void mips_reg_load(int reg_id, IrOprand* op) {}

void mips_var_init() {
    mips_var_list = (mips_var_t*)malloc(sizeof(mips_var_t));
    mips_var_list->op = NULL;
    mips_var_list->offset = 0;
    mips_var_list->prev = mips_var_list->next = mips_var_list;
}

mips_var_t* mips_var_new(IrOprand* op) {
    mips_var_t* ret = (mips_var_t*)malloc(sizeof(mips_var_t));

    if (op->type != OP_VAR && op->type != OP_TEMP)
        assert(0);

    ret->op = op;

    ret->offset = 0;

    mips_var_list->next->prev = ret;
    ret->next = mips_var_list->next;
    ret->prev = mips_var_list;
    mips_var_list->next = ret;

    return ret;
}

mips_var_t* mips_var_get(IrOprand* op) {
    mips_var_t* ptr = mips_var_list;
    while (ptr != mips_var_list) {
        if (ptr->op->type != op->type) {
            continue;
        }
        if (op->type == OP_TEMP) {
            if (op->data_int == ptr->op->data_int) {
                return ptr;
            }
        } else if (op->type == OP_VAR) {
            if (strcmp(op->data_str, ptr->op->data_str) == 0) {
                return ptr;
            }
        }
    }

    return NULL;
}

void mips_print(IrCode* code_list) {
    mips_reg_init();
    mips_var_init();
}