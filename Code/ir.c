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

void ircode_opt(IrCode* tail) {
    ircode_opt_useless(tail);
    ircode_opt_address(tail);
    ircode_opt_eval(tail);
}

void ircode_opt_useless(IrCode* tail) {
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

void ircode_opt_address(IrCode* tail) {}

void ircode_opt_eval(IrCode* tail) {}
