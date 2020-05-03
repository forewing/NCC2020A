#include "ir.h"

IrCode* ircode_list = NULL;
int tmpvar_num = 1;
int label_num = 1;
int compst_num = 1;

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
    if (pos->prev)
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
            MALLOC_PRINTF(ret, "f_%s", op->data_str);
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

void IrCode_print(FILE* fp, IrCode* root) {
    IrCode* ptr = root;
    while (ptr) {
        const char* x = IrOprand_print(ptr->x);
        const char* y = IrOprand_print(ptr->y);
        const char* z = IrOprand_print(ptr->z);
        const char* relop = "INVALID";

        switch (ptr->type) {
            case CODE_NOP:
                break;
            case CODE_LABEL:
                fprintf(fp, "LABEL %s:\n", x);
                break;
            case CODE_FUNC:
                fprintf(fp, "FUNCTION %s:\n", x);
                break;
            case CODE_ASSIGN:
                fprintf(fp, "%s := %s\n", x, y);
                break;
            case CODE_ADD:
                fprintf(fp, "%s := %s + %s\n", x, y, z);
                break;
            case CODE_SUB:
                fprintf(fp, "%s := %s - %s\n", x, y, z);
                break;
            case CODE_MUL:
                fprintf(fp, "%s := %s * %s\n", x, y, z);
                break;
            case CODE_DIV:
                fprintf(fp, "%s := %s / %s\n", x, y, z);
                break;
            case CODE_GETADDR:
                fprintf(fp, "%s := &%s\n", x, y);
                break;
            case CODE_GETDATA:
                fprintf(fp, "%s := *%s\n", x, y);
                break;
            case CODE_SETDATA:
                fprintf(fp, "*%s := %s\n", x, y);
                break;
            case CODE_GOTO:
                fprintf(fp, "GOTO %s\n", x);
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
                fprintf(fp, "IF %s [relop] %s GOTO %s\n", x, y, z);
                break;
            case CODE_RET:
                fprintf(fp, "RETURN %s\n", x);
                break;
            case CODE_DEC:
                fprintf(fp, "DEC %s %d\n", x, ptr->data_int);
                break;
            case CODE_ARG:
                fprintf(fp, "ARG %s\n", x);
                break;
            case CODE_CALL:
                fprintf(fp, "%s := CALL %s\n", x, y);
                break;
            case CODE_PARAM:
                fprintf(fp, "PARAM %s\n", x);
                break;
            case CODE_READ:
                fprintf(fp, "READ %s\n", x);
                break;
            case CODE_WRITE:
                fprintf(fp, "WRITE %s\n", x);
                break;
            default:
                break;
        }
    }
}
