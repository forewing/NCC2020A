#include "symbol.h"

const SymbolNode int_entity =
    {.data_int = 0, "CONST_INT_ENTITY", 0, TYPE_INT, 1, 0};
const SymbolNode float_entity =
    {.data_float = 0.0, "CONST_FLOAT_ENTITY", 0, TYPE_FLOAT, 1, 0};
const SymbolNode void_entity =
    {.data_struct = {NULL, 0}, "CONST_VOID_ENTITY", 0, TYPE_STRUCT, 1, 0};

int typeEqual(SymbolNode* a, SymbolNode* b) {
    if (!a || !b)
        return 0;
    if (a->type == TYPE_INVALID || b->type == TYPE_INVALID)
        return 1;
    if (a->type != b->type)
        return 0;

    if (a->type == TYPE_ARRAY) {
        if (a->data_array.dimen != b->data_array.dimen)
            return 0;
        return typeEqual(a->data_array.next, b->data_array.next);
    } else if (a->type == TYPE_STRUCT) {
        if (a->data_struct.size != b->data_struct.size)
            return 0;
        for (int i = 0; i < a->data_struct.size; i++)
            if (!typeEqual(a->data_struct.types[i], b->data_struct.types[i]))
                return 0;
        return 1;
    } else if (a->type == TYPE_FUNC) {
        if (typeEqual(a->data_func.ret, b->data_func.ret) &&
            typeEqual(a->data_func.args, b->data_func.args))
            return 1;
        return 0;
    }

    return 1;
}

SymbolNode* type_new() {
    SymbolNode* ret = (SymbolNode*)malloc(sizeof(SymbolNode));
    ret->name = NULL;
    ret->offset = 0;
    ret->type = TYPE_INVALID;
    ret->is_right = 0;
    ret->line = 0;
    return ret;
}

SymbolNode* type_new_invalid() {
    SymbolNode* ret = type_new();
    ret->type = TYPE_INVALID;
    return ret;
}

SymbolNode* type_new_int(int value) {
    SymbolNode* ret = type_new();
    ret->type = TYPE_INT;
    ret->data_int = value;
    return ret;
}

SymbolNode* type_new_float(float value) {
    SymbolNode* ret = type_new();
    ret->type = TYPE_FLOAT;
    ret->data_float = value;
    return ret;
}

SymbolNode* type_new_array(SymbolNode* next) {
    SymbolNode* ret = type_new();
    ret->type = TYPE_ARRAY;
    ret->data_array.next = next;
    ret->data_array.size = 0;
    if (next->type == TYPE_ARRAY)
        ret->data_array.dimen = next->data_array.dimen + 1;
    else
        ret->data_array.dimen = 1;
    return ret;
}

SymbolNode* type_new_struct(int size) {
    SymbolNode* ret = type_new();
    ret->type = TYPE_STRUCT;
    ret->data_struct.size = size;
    ret->data_struct.types = (SymbolNode**)malloc(sizeof(SymbolNode) * size);
    return ret;
}

SymbolNode* type_new_func(SymbolNode* ret, SymbolNode* args) {
    SymbolNode* tmp = type_new();
    tmp->type = TYPE_FUNC;
    tmp->data_func.ret = ret;
    tmp->data_func.args = args;
    return tmp;
}

int type_free(SymbolNode* node) {
    // if (!node)
    //     return -1;

    // if (node->type == TYPE_STRUCT) {
    //     for (int i = 0; i < node->data_struct.size; i++)
    //         type_free(node->data_struct.types[i]);
    // } else if (node->type == TYPE_FUNC) {
    //     type_free(node->data_func.ret);
    //     type_free(node->data_func.args);
    // }

    // FREE(node);
    return 0;
}

SymbolNode* type_dup(SymbolNode* type) {
    SymbolNode* ret = (SymbolNode*)malloc(sizeof(SymbolNode));
    memcpy(ret, type, sizeof(SymbolNode));
    return ret;
}

SymbolNode* type_dup_right(SymbolNode* type) {
    SymbolNode* ret = type_dup(type);
    ret->is_right = 1;
    return ret;
}

SymbolNode* type_dup_left(SymbolNode* type) {
    SymbolNode* ret = type_dup(type);
    ret->is_right = 0;
    return ret;
}