#include "ir.h"

IrCode* ircode_list = NULL;
int tmpvar_num = 0;
int label_num = 0;
int compst_num = 0;

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
