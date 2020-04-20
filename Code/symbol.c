#include "symbol.h"

const SymNode int_entity =
    {.data_int = 0, "CONST_INT_ENTITY", 0, TYPE_INT, 1, 0};
const SymNode float_entity =
    {.data_float = 0.0, "CONST_FLOAT_ENTITY", 0, TYPE_FLOAT, 1, 0};
const SymNode void_entity =
    {.data_struct = {NULL, 0}, "CONST_VOID_ENTITY", 0, TYPE_STRUCT, 1, 0};
const SymNode invalid_entity =
    {.data_int = 0, "CONST_INVALID_ENTITY", 0, TYPE_INVALID, 0, 0};

int typeEqual(const SymNode* a, const SymNode* b) {
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

SymNode* type_new() {
    SymNode* ret = (SymNode*)malloc(sizeof(SymNode));
    ret->name = NULL;
    ret->offset = 0;
    ret->size = 0;
    ret->type = TYPE_INVALID;
    ret->is_right = 0;
    ret->line = 0;
    return ret;
}

SymNode* type_new_invalid() {
    SymNode* ret = type_new();
    ret->type = TYPE_INVALID;
    return ret;
}

SymNode* type_new_int(int value) {
    SymNode* ret = type_new();
    ret->type = TYPE_INT;
    ret->size = 4;
    ret->data_int = value;
    return ret;
}

SymNode* type_new_float(float value) {
    SymNode* ret = type_new();
    ret->type = TYPE_FLOAT;
    ret->size = 4;
    ret->data_float = value;
    return ret;
}

SymNode* type_new_array(SymNode* next) {
    SymNode* ret = type_new();
    ret->type = TYPE_ARRAY;
    ret->data_array.next = next;
    ret->data_array.size = 0;
    if (next->type == TYPE_ARRAY)
        ret->data_array.dimen = next->data_array.dimen + 1;
    else
        ret->data_array.dimen = 1;
    return ret;
}

SymNode* type_new_struct(int size) {
    SymNode* ret = type_new();
    ret->type = TYPE_STRUCT;
    ret->data_struct.size = size;
    ret->data_struct.types = (SymNode**)malloc(sizeof(SymNode) * size);
    return ret;
}

SymNode* type_new_func(SymNode* ret, SymNode* args) {
    SymNode* tmp = type_new();
    tmp->type = TYPE_FUNC;
    tmp->data_func.ret = ret;
    tmp->data_func.args = args;
    return tmp;
}

int type_free(SymNode* node) {
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

SymNode* type_dup(const SymNode* type, int right) {
    if (!type)
        return type_new_invalid();
    SymNode* ret = (SymNode*)malloc(sizeof(SymNode));
    memcpy(ret, type, sizeof(SymNode));

    if (right == 0) {
        ret->is_right = 0;
    } else if (right == 1) {
        ret->is_right = 1;
    }

    if (ret->type == TYPE_ARRAY) {
        ret->data_array.next = type_dup(ret->data_array.next, right);
    } else if (ret->type == TYPE_STRUCT) {
        ret->data_struct.types =
            (SymNode**)malloc(sizeof(SymNode*) * ret->data_struct.size);
        for (int i = 0; i < ret->data_struct.size; i++) {
            ret->data_struct.types[i] =
                type_dup(type->data_struct.types[i], right);
        }
    } else if (ret->type == TYPE_FUNC) {
        ret->data_func.ret = type_dup(type->data_func.ret, right);
        ret->data_func.args = type_dup(type->data_func.args, right);
    }

    return ret;
}

SymNode* type_dup_left(const SymNode* type) {
    return type_dup(type, 0);
}
SymNode* type_dup_right(const SymNode* type) {
    return type_dup(type, 1);
}
