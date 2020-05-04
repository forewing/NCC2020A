#include "ir.h"

IrCode* ircode_list = NULL;
int tmpvar_num = 1;
int label_num = 1;
int compst_num = 1;

int tmpvar_new() {
    return tmpvar_num++;
}
int label_new() {
    return label_num++;
}

IrOprand* IrOprand_new(int type) {
    IrOprand* ret = (IrOprand*)malloc(sizeof(IrOprand));
    ret->type = type;
    ret->data_str = NULL;
    return ret;
}

IrOprand* IrOprand_new_int(int type, int data) {
    IrOprand* ret = IrOprand_new(type);
    ret->data_int = data;
    return ret;
}

IrOprand* IrOprand_new_str(int type, const char* data) {
    IrOprand* ret = IrOprand_new(type);
    ret->data_str = data;
    return ret;
}

IrOprand* IrOprand_new_op(int type, IrOprand* data) {
    IrOprand* ret = IrOprand_new(type);
    ret->data_op = data;
    return ret;
}

IrCode* IrCode_new(int type, int data, IrOprand* x, IrOprand* y, IrOprand* z) {
    IrCode* ret = (IrCode*)malloc(sizeof(IrCode));

    ret->prev = NULL;
    ret->next = NULL;

    ret->type = type;
    ret->data_int = data;

    ret->x = x;
    ret->y = y;
    ret->z = z;

    return ret;
}

void IrCode_insert(IrCode* pos, IrCode* elem) {
    elem->next = pos;
    elem->prev = pos->prev;
    pos->prev->next = elem;
    pos->prev = elem;
}

void IrCode_delete(IrCode* pos) {
    pos->prev->next = pos->next;
    pos->next->prev = pos->prev;
}

#define MALLOC_PRINTF(target, ...)                          \
    {                                                       \
        size_t needed = snprintf(NULL, 0, __VA_ARGS__) + 1; \
        target = (char*)malloc(needed);                     \
        sprintf(target, __VA_ARGS__);                       \
    }

const char* IrOprand_print(IrOprand* op) {
    if (!op)
        return "INVALID";
    char* ret = NULL;
    switch (op->type) {
        case OP_NULL:
            return "INVALID";
            break;
        case OP_CONST:
            MALLOC_PRINTF(ret, "#%d", op->data_int);
            break;
        case OP_VAR:
            MALLOC_PRINTF(ret, "v_%s", op->data_str);
            break;
        case OP_GETADDR:
            MALLOC_PRINTF(ret, "&%s", IrOprand_print(op->data_op));
            break;
        case OP_GETDATA:
            MALLOC_PRINTF(ret, "*%s", IrOprand_print(op->data_op));
            break;
        case OP_FUNC:
            if (!strcmp(op->data_str, "main")) {
                MALLOC_PRINTF(ret, "%s", op->data_str);
            } else {
                MALLOC_PRINTF(ret, "f_%s", op->data_str);
            }
            break;
        case OP_LABEL:
            MALLOC_PRINTF(ret, "l_%d", op->data_int);
            break;
        case OP_TEMP:
            MALLOC_PRINTF(ret, "t_%d", op->data_int);
            break;
        default:
            return "INVALID";
            break;
    }
    return ret;
}

void IrCode_print(FILE* fp, IrCode* tail) {
    IrCode* ptr = tail->next;
    while (ptr != tail) {
        const char* x = IrOprand_print(ptr->x);
        const char* y = IrOprand_print(ptr->y);
        const char* z = IrOprand_print(ptr->z);
        const char* relop = "INVALID";

        switch (ptr->type) {
            case CODE_NOP:
                break;
            case CODE_LABEL:
                fprintf(fp, "LABEL\t%s :\n", x);
                break;
            case CODE_FUNC:
                fprintf(fp, "FUNCTION\t%s :\n", x);
                break;
            case CODE_ASSIGN:
                fprintf(fp, "%s\t:=\t%s\n", x, y);
                break;
            case CODE_ADD:
                fprintf(fp, "%s\t:=\t%s\t+\t%s\n", x, y, z);
                break;
            case CODE_SUB:
                fprintf(fp, "%s\t:=\t%s\t-\t%s\n", x, y, z);
                break;
            case CODE_MUL:
                fprintf(fp, "%s\t:=\t%s\t*\t%s\n", x, y, z);
                break;
            case CODE_DIV:
                fprintf(fp, "%s\t:=\t%s\t/\t%s\n", x, y, z);
                break;
            case CODE_GOTO:
                fprintf(fp, "GOTO\t%s\n", x);
                break;
            case CODE_GOCOND:
                if (ptr->data_int == RELOP_EQ)
                    relop = "==";
                else if (ptr->data_int == RELOP_GE)
                    relop = ">=";
                else if (ptr->data_int == RELOP_GT)
                    relop = ">";
                else if (ptr->data_int == RELOP_LE)
                    relop = "<=";
                else if (ptr->data_int == RELOP_LT)
                    relop = "<";
                else if (ptr->data_int == RELOP_NE)
                    relop = "!=";
                else
                    relop = "INVALID";
                fprintf(fp, "IF\t%s\t%s\t%s\tGOTO\t%s\n", x, relop, y, z);
                break;
            case CODE_RET:
                fprintf(fp, "RETURN\t%s\n", x);
                break;
            case CODE_DEC:
                fprintf(fp, "DEC\t%s\t%d\n", x, ptr->data_int);
                break;
            case CODE_ARG:
                fprintf(fp, "ARG\t%s\n", x);
                break;
            case CODE_CALL:
                fprintf(fp, "%s\t:=\tCALL %s\n", x, y);
                break;
            case CODE_PARAM:
                fprintf(fp, "PARAM\t%s\n", x);
                break;
            case CODE_READ:
                fprintf(fp, "READ\t%s\n", x);
                break;
            case CODE_WRITE:
                fprintf(fp, "WRITE\t%s\n", x);
                break;
            default:
                break;
        }

        ptr = ptr->next;
    }
}

void ircode_opt_zero_tmp(IrCode* tail);
void ircode_opt_exist_once(IrCode* tail);
void ircode_opt_assign_once(IrCode* tail);
void ircode_opt_address(IrCode* tail);
void ircode_opt_call(IrCode* tail);
void ircode_opt_eval(IrCode* tail);
void ircode_opt_eval_zeros(IrCode* tail);

int* tmpvar_int_list = NULL;
void** tmpvar_ptr_list = NULL;
void** tmpvar_ptr_list_2 = NULL;

void ircode_opt(IrCode* tail) {
    tmpvar_int_list = (int*)malloc(sizeof(int) * tmpvar_num);
    tmpvar_ptr_list = (void**)malloc(sizeof(void*) * tmpvar_num);
    tmpvar_ptr_list_2 = (void**)malloc(sizeof(void*) * tmpvar_num);
    ircode_opt_zero_tmp(tail);
    for (int i = 0; i < 5; i++) {
        ircode_opt_exist_once(tail);
        ircode_opt_assign_once(tail);
        ircode_opt_address(tail);
        // ircode_opt_call(tail);
        ircode_opt_eval(tail);
        ircode_opt_eval_zeros(tail);
    }
}

void ircode_opt_zero_tmp(IrCode* tail) {
    IrCode* ptr = tail->next;
    while (ptr != tail) {
        if ((ptr->type == CODE_ASSIGN || ptr->type == CODE_ADD || ptr->type == CODE_SUB || ptr->type == CODE_MUL ||
             ptr->type == CODE_DIV) &&
            ptr->x->type == OP_TEMP && ptr->x->data_int == 0) {
            ptr = ptr->next;
            IrCode_delete(ptr->prev);
        } else {
            ptr = ptr->next;
        }
    }
}

int ircode_opt_get_tmp_id(IrOprand* op) {
    if (!op)
        return -1;
    if (op->type == OP_TEMP)
        return op->data_int;
    if (op->type == OP_GETADDR || op->type == OP_GETDATA)
        return ircode_opt_get_tmp_id(op->data_op);
    return -1;
}

void ircode_opt_exist_once(IrCode* tail) {
    if (!tmpvar_int_list)
        return;

    for (int i = 0; i < tmpvar_num; i++)
        tmpvar_int_list[i] = 0;

    IrCode* ptr = tail->next;
    while (ptr != tail) {
        int tmp_id[3];
        tmp_id[0] = ircode_opt_get_tmp_id(ptr->x);
        tmp_id[1] = ircode_opt_get_tmp_id(ptr->y);
        tmp_id[2] = ircode_opt_get_tmp_id(ptr->z);
        for (int i = 0; i < 3; i++) {
            if (tmp_id[i] != -1) {
                tmpvar_int_list[tmp_id[i]]++;
            }
        }
        ptr = ptr->next;
    }

    ptr = tail->next;
    while (ptr != tail) {
        if ((ptr->type == CODE_ASSIGN || ptr->type == CODE_ADD || ptr->type == CODE_SUB || ptr->type == CODE_MUL ||
             ptr->type == CODE_DIV) &&
            ptr->x->type == OP_TEMP && tmpvar_int_list[ptr->x->data_int] <= 1) {
            ptr = ptr->next;
            IrCode_delete(ptr->prev);
        } else {
            ptr = ptr->next;
        }
    }
}

void ircode_opt_assign_once(IrCode* tail) {
    if (!tmpvar_ptr_list || !tmpvar_int_list || !tmpvar_ptr_list_2)
        return;

    for (int i = 0; i < tmpvar_num; i++) {
        tmpvar_int_list[i] = 0;
        tmpvar_ptr_list[i] = NULL;
    }

    IrCode* ptr = tail->next;
    while (ptr != tail) {
        if ((ptr->type == CODE_ASSIGN || ptr->type == CODE_ADD || ptr->type == CODE_SUB || ptr->type == CODE_MUL ||
             ptr->type == CODE_DIV || ptr->type == CODE_CALL) &&
            ptr->x->type == OP_TEMP) {
            tmpvar_int_list[ptr->x->data_int]++;
            if (ptr->type == CODE_ASSIGN) {
                tmpvar_ptr_list[ptr->x->data_int] = ptr->y;
                tmpvar_ptr_list_2[ptr->x->data_int] = ptr;
            }
        }

        ptr = ptr->next;
    }

    for (int i = 0; i < tmpvar_num; i++) {
        if (tmpvar_int_list[i] != 1 || !tmpvar_ptr_list[i])
            continue;

        IrCode_delete(tmpvar_ptr_list_2[i]);

        IrCode* ptr = tail->next;
        while (ptr != tail) {
            ptr = ptr->next;
            IrOprand* tmp[3];
            tmp[0] = ptr->x;
            tmp[1] = ptr->y;
            tmp[2] = ptr->z;
            for (int j = 0; j < 3; j++) {
                if (!tmp[j])
                    continue;
                IrOprand* op = tmp[j];
                while (op->type == OP_GETDATA || op->type == OP_GETADDR)
                    op = op->data_op;
                if (op->type == OP_TEMP && op->data_int == i)
                    memcpy(op, tmpvar_ptr_list[i], sizeof(IrOprand));
            }
        }
    }
}

void ircode_opt_address_once(IrOprand* op) {
    if (!op)
        return;

    if (op->type == OP_GETADDR) {
        IrOprand* op2 = op->data_op;
        if (op2 && op2->type == OP_GETDATA && op2->data_op) {
            memcpy(op, op2->data_op, sizeof(IrOprand));
            ircode_opt_address_once(op);
        }
    } else if (op->type == OP_GETDATA) {
        IrOprand* op2 = op->data_op;
        if (op2 && op2->type == OP_GETADDR && op2->data_op) {
            memcpy(op, op2->data_op, sizeof(IrOprand));
            ircode_opt_address_once(op);
        }
    }
}

void ircode_opt_address(IrCode* tail) {
    IrCode* ptr = tail->next;
    while (ptr != tail) {
        ircode_opt_address_once(ptr->x);
        ircode_opt_address_once(ptr->y);
        ircode_opt_address_once(ptr->z);
        ptr = ptr->next;
    }
}

void ircode_opt_call(IrCode* tail) {
    if (!tmpvar_int_list || !tmpvar_ptr_list || !tmpvar_ptr_list_2)
        return;

    IrCode* ptr = tail->next;
    while (ptr != tail) {
        ptr = ptr->next;
    }
}

void ircode_opt_eval(IrCode* tail) {
    IrCode* ptr = tail->next;

    while (ptr != tail) {
        if (ptr->type == CODE_ADD || ptr->type == CODE_SUB || ptr->type == CODE_MUL || ptr->type == CODE_DIV) {
            if (ptr->y->type == OP_CONST && ptr->z->type == OP_CONST) {
                int result = 0;
                if (ptr->type == CODE_ADD)
                    result = ptr->y->data_int + ptr->z->data_int;
                else if (ptr->type == CODE_SUB)
                    result = ptr->y->data_int - ptr->z->data_int;
                else if (ptr->type == CODE_MUL)
                    result = ptr->y->data_int * ptr->z->data_int;
                else if (ptr->type == CODE_DIV)
                    result = ptr->y->data_int / ptr->z->data_int;

                ptr->type = CODE_ASSIGN;

                ptr->y->data_int = result;
                ptr->z = NULL;
            }
        }
        ptr = ptr->next;
    }
}

void ircode_opt_eval_zeros(IrCode* tail) {
    IrCode* ptr = tail->next;
    while (ptr != tail) {
        if (ptr->type != CODE_ADD && ptr->type != CODE_SUB && ptr->type != CODE_MUL && ptr->type != CODE_DIV) {
            ptr = ptr->next;
            continue;
        }

        if (ptr->y->type == OP_CONST && ptr->y->data_int == 0 && ptr->type == CODE_ADD) {
            // 0 + x = x
            ptr->type = CODE_ASSIGN;
            memcpy(ptr->y, ptr->z, sizeof(IrOprand));
            ptr->z = NULL;
        } else if (ptr->z->type == OP_CONST && ptr->z->data_int == 0 &&
                   (ptr->type == CODE_ADD || ptr->type == CODE_SUB)) {
            // x +- 0 = x
            ptr->type = CODE_ASSIGN;
            ptr->z = NULL;
        } else if ((ptr->type == CODE_MUL) && ((ptr->y->type == OP_CONST && ptr->y->data_int == 0) ||
                                               (ptr->z->type == OP_CONST && ptr->z->data_int == 0))) {
            // 0 * x = x * 0 = 0
            ptr->type = CODE_ASSIGN;
            ptr->y->type = OP_CONST;
            ptr->y->data_int = 0;
            ptr->z = NULL;
        } else if ((ptr->type == CODE_MUL || ptr->type == CODE_DIV) &&
                   (ptr->z->type == OP_CONST && ptr->z->data_int == 1)) {
            // x */ 1 = x
            ptr->type = CODE_ASSIGN;
            ptr->z = NULL;
        } else if (ptr->type == CODE_MUL && ptr->y->type == OP_CONST && ptr->y->data_int == 1) {
            // 1 * x = x
            ptr->type = CODE_ASSIGN;
            memcpy(ptr->y, ptr->z, sizeof(IrOprand));
            ptr->z = NULL;
        }

        ptr = ptr->next;
    }
}