#include "ir.h"

IrCode* ircode_list = NULL;
int tmpvar_num = 0;
int label_num = 0;

IrOprand* IrOprand_new() {
    IrOprand* ret = (IrOprand*)malloc(sizeof(IrOprand));
    ret->type = OP_NULL;
    ret->data_str = NULL;
    return ret;
}

IrCode* IrCode_new() {
    IrCode* ret = (IrCode*)malloc(sizeof(IrCode));

    ret->prev = NULL;
    ret->next = NULL;

    ret->type = CODE_NOP;

    ret->x = NULL;
    ret->y = NULL;
    ret->z = NULL;

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

