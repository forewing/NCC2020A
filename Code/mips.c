#include "mips.h"

char* mips_reg_name[MIPS_REG_NR] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2",
                                    "$t3",   "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5",
                                    "$s6",   "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};

mips_reg_t mips_regs[MIPS_REG_NR];
mips_var_t* mips_var_list;

int mips_sp_offset;
int mips_arg_nr;
int mips_param_nr;
FILE* mips_fp;

void mips_reg_init() {
    for (int i = 0; i < MIPS_REG_NR; i++) {
        mips_regs[i].type = i;
        mips_regs[i].var = NULL;
    }
}

void mips_reg_load(int reg_id, IrOprand* op) {
    fprintf(mips_fp, "# load %s <- %s\n", mips_reg_name[reg_id], IrOprand_print(op));

    mips_regs[reg_id].var = NULL;

    if (op->type == OP_CONST) {
        // const
        fprintf(mips_fp, "    li %s, %d\n", mips_reg_name[reg_id], op->data_int);
        return;
    }

    IrOprand* op_real = op;
    if (op->type == OP_GETADDR || op->type == OP_GETDATA)
        op_real = op->data_op;

    mips_var_t* var = mips_var_get(op_real);
    if (!var) {
        // alloc
        fprintf(mips_fp, "# no exist, alloc\n");
        var = mips_var_new(op_real);
        mips_sp_offset -= 4;
        fprintf(mips_fp, "    addi $sp, $sp, -4\n");
        var->offset = mips_sp_offset;
    }

    if (op->type == OP_GETADDR) {
        fprintf(mips_fp, "    addi %s, $fp, %d\n", mips_reg_name[reg_id], var->offset);
    } else if (op->type == OP_GETDATA) {
        fprintf(mips_fp, "    lw $s0, %d($fp)\n", var->offset);
        fprintf(mips_fp, "    lw %s, 0($s0)\n", mips_reg_name[reg_id]);
    } else {
        // temp, var
        fprintf(mips_fp, "    lw %s, %d($fp)\n", mips_reg_name[reg_id], var->offset);
    }

    mips_regs[reg_id].var = var;
}

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
    mips_var_t* ptr = mips_var_list->next;
    while (ptr != mips_var_list) {
        if (ptr->op->type == op->type) {
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
        ptr = ptr->next;
    }

    return NULL;
}

void mips_print(FILE* fp, IrCode* code_list) {
    mips_reg_init();
    mips_var_init();

    mips_fp = fp;

    fprintf(mips_fp,
            ".data\n"
            // "_prompt: .asciiz \" \"\n"
            "_prompt: .asciiz \"Enter an integer:\"\n"
            "_ret: .asciiz \"\\n\"\n"
            ".globl main\n"
            ".text\n"
            // "read:\n"
            // "    li $v0, 4\n"
            // "    la $a0, _prompt\n"
            // "    syscall\n"
            // "    li $v0, 5\n"
            // "    syscall\n"
            // "    jr $ra\n"
            // "write:\n"
            // "    li $v0, 1\n"
            // "    syscall\n"
            // "    li $v0, 4\n"
            // "    la $a0, _ret\n"
            // "    syscall\n"
            // "    move $v0, $0\n"
            // "    jr $ra\n"
    );

    IrCode* ptr = code_list->next;
    while (ptr != code_list) {
        fprintf(fp, "# ");
        IrCode_print_once(fp, ptr);

        switch (ptr->type) {
            case CODE_LABEL:
                mips_print_LABEL(ptr);
                break;
            case CODE_FUNC:
                mips_print_FUNC(ptr);
                break;
            case CODE_ASSIGN:
                mips_print_ASSIGN(ptr);
                break;
            case CODE_ADD:
                mips_print_ADD(ptr);
                break;
            case CODE_SUB:
                mips_print_SUB(ptr);
                break;
            case CODE_MUL:
                mips_print_MUL(ptr);
                break;
            case CODE_DIV:
                mips_print_DIV(ptr);
                break;
            case CODE_GOTO:
                mips_print_GOTO(ptr);
                break;
            case CODE_GOCOND:
                mips_print_GOCOND(ptr);
                break;
            case CODE_RET:
                mips_print_RET(ptr);
                break;
            case CODE_DEC:
                mips_print_DEC(ptr);
                break;
            case CODE_ARG:
                mips_print_ARG(ptr);
                break;
            case CODE_CALL:
                mips_print_CALL(ptr);
                break;
            case CODE_PARAM:
                mips_print_PARAM(ptr);
                break;
            case CODE_READ:
                mips_print_READ(ptr);
                break;
            case CODE_WRITE:
                mips_print_WRITE(ptr);
                break;
            default:
                break;
        }
        ptr = ptr->next;
    }
}

void mips_printf_setvar(IrOprand* dst, int src) {
    mips_reg_load(MIPS_REG_S1, dst);
    fprintf(mips_fp, "# setvar %s <- %s\n", IrOprand_print(dst), mips_reg_name[src]);
    if (dst->type == OP_GETDATA) {
        fprintf(mips_fp, "    lw $s1, %d($fp)\n", mips_regs[MIPS_REG_S1].var->offset);
        fprintf(mips_fp, "    sw %s, 0($s1)\n", mips_reg_name[src]);
    } else {
        fprintf(mips_fp, "    sw %s, %d($fp)\n", mips_reg_name[src], mips_regs[MIPS_REG_S1].var->offset);
    }
}

void mips_print_LABEL(IrCode* code) {
    fprintf(mips_fp, "l_%d:\n", code->x->data_int);
}

void mips_print_FUNC(IrCode* code) {
    if (strcmp(code->x->data_str, "main") == 0) {
        fprintf(mips_fp, "main:\n");
    } else {
        fprintf(mips_fp, "f_%s:\n", code->x->data_str);
    }
    // args
    // old ra
    // old fp <- fp
    fprintf(mips_fp,
            "    addi $sp, $sp, -4\n"
            "    sw $ra, 0($sp)\n"
            "    addi $sp, $sp, -4\n"
            "    sw $fp, 0($sp)\n"
            "    move $fp, $sp\n");

    mips_sp_offset = 0;
    mips_arg_nr = 0;
    mips_param_nr = 0;
}

void mips_print_ASSIGN(IrCode* code) {
    mips_reg_load(MIPS_REG_T0, code->y);
    mips_printf_setvar(code->x, MIPS_REG_T0);
}

void mips_print_ADD(IrCode* code) {
    mips_reg_load(MIPS_REG_T1, code->y);
    mips_reg_load(MIPS_REG_T2, code->z);

    fprintf(mips_fp, "    add $t0, $t1, $t2\n");
    mips_printf_setvar(code->x, MIPS_REG_T0);
}

void mips_print_SUB(IrCode* code) {
    mips_reg_load(MIPS_REG_T1, code->y);
    mips_reg_load(MIPS_REG_T2, code->z);

    fprintf(mips_fp, "    sub $t0, $t1, $t2\n");
    mips_printf_setvar(code->x, MIPS_REG_T0);
}

void mips_print_MUL(IrCode* code) {
    mips_reg_load(MIPS_REG_T1, code->y);
    mips_reg_load(MIPS_REG_T2, code->z);

    fprintf(mips_fp, "    mul $t0, $t1, $t2\n");
    mips_printf_setvar(code->x, MIPS_REG_T0);
}

void mips_print_DIV(IrCode* code) {
    mips_reg_load(MIPS_REG_T1, code->y);
    mips_reg_load(MIPS_REG_T2, code->z);

    fprintf(mips_fp, "    div $t1, $t2\n");
    fprintf(mips_fp, "    mflo $t0\n");
    mips_printf_setvar(code->x, MIPS_REG_T0);
}

void mips_print_GOTO(IrCode* code) {
    fprintf(mips_fp, "    j l_%d\n", code->x->data_int);
}

void mips_print_GOCOND(IrCode* code) {
    mips_reg_load(MIPS_REG_T0, code->x);
    mips_reg_load(MIPS_REG_T1, code->y);

    const char* intr = "INVALID";

    if (code->data_int == RELOP_EQ) {
        intr = "beq";
    } else if (code->data_int == RELOP_NE) {
        intr = "bne";
    } else if (code->data_int == RELOP_LE) {
        intr = "ble";
    } else if (code->data_int == RELOP_GE) {
        intr = "bge";
    } else if (code->data_int == RELOP_LT) {
        intr = "blt";
    } else if (code->data_int == RELOP_GT) {
        intr = "bgt";
    }

    fprintf(mips_fp, "    %s $t0, $t1, l_%d\n", intr, code->z->data_int);
}

void mips_print_RET(IrCode* code) {
    // args         (1) sp go back here
    // old ra       (2) load ra
    // old fp <- fp (3) load fp
    mips_reg_load(MIPS_REG_V0, code->x);
    fprintf(mips_fp,
            "    addi $sp, $fp, 8\n"
            "    lw $ra 4($fp)\n"
            "    lw $fp 0($fp)\n"
            "    jr $ra\n");
}

void mips_print_DEC(IrCode* code) {
    mips_var_t* var = mips_var_new(code->x);
    mips_sp_offset -= code->data_int;
    fprintf(mips_fp, "    addi $sp, $sp, %d\n", -code->data_int);
    var->offset = mips_sp_offset;
}

void mips_print_ARG(IrCode* code) {
    // arg...
    // arg2
    // arg1

    mips_reg_load(MIPS_REG_T0, code->x);
    fprintf(mips_fp, "    addi $sp, $sp, -4\n");
    fprintf(mips_fp, "    sw $t0, 0($sp)\n");

    mips_sp_offset -= 4;
    mips_arg_nr++;
}

void mips_print_CALL(IrCode* code) {
    if (strcmp(code->y->data_str, "main") == 0) {
        fprintf(mips_fp, "    jal main\n");
    } else {
        fprintf(mips_fp, "    jal f_%s\n", code->y->data_str);
    }

    // remove args
    fprintf(mips_fp, "    addi $sp, $sp, %d\n", mips_arg_nr * 4);
    mips_sp_offset += mips_arg_nr * 4;
    mips_arg_nr = 0;

    // v0 holds return value
    mips_printf_setvar(code->x, MIPS_REG_V0);
}

void mips_print_PARAM(IrCode* code) {
    // arg2 <- fp + 8 + 4 * 1
    // arg1 <- fp + 8
    // old ra
    // old fp <- fp

    mips_var_t* var = mips_var_new(code->x);
    var->offset = mips_sp_offset + 8 + mips_param_nr * 4;
    mips_param_nr++;
}

void mips_print_READ(IrCode* code) {
    fprintf(mips_fp,
            "    li $v0, 4\n"
            "    la $a0, _prompt\n"
            "    syscall\n"
            "    li $v0, 5\n"
            "    syscall\n");

    // data at $v0
    mips_printf_setvar(code->x, MIPS_REG_V0);
}

void mips_print_WRITE(IrCode* code) {
    mips_reg_load(MIPS_REG_A0, code->x);

    fprintf(mips_fp,
            "    li $v0, 1\n"
            "    syscall\n"
            "    li $v0, 4\n"
            "    la $a0, _ret\n"
            "    syscall\n");
}
