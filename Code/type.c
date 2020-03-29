#include "type.h"
#include <stdlib.h>
#include <string.h>
#include "helper.h"

const TypeNode* int_type_entity;
const TypeNode* float_type_entity;
const TypeNode* void_type_entity;

int typeEqual(TypeNode* a, TypeNode* b) {
    if (!a || !b)
        return 0;
    if (a->type == TYPE_INVALID || b->type == TYPE_INVALID)
        return 1;
    if (a->type != b->type)
        return 0;
    if (a->dimen != b->dimen)
        return 0;

    if (a->type == TYPE_STRUCT) {
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

    if (a->type == b->type)
        return 1;

    return 0;
}

TypeNode* type_new_invalid() {
    TypeNode* ret = (TypeNode*)malloc(sizeof(TypeNode));
    ret->type = TYPE_INVALID;
    ret->is_right = 0;
    return ret;
}

TypeNode* type_new_int(int value) {
    TypeNode* ret = (TypeNode*)malloc(sizeof(TypeNode));
    ret->type = TYPE_INT;
    ret->data_int = value;
    ret->is_right = 0;
    ret->dimen = 0;
    return ret;
}

TypeNode* type_new_float(float value) {
    TypeNode* ret = (TypeNode*)malloc(sizeof(TypeNode));
    ret->type = TYPE_FLOAT;
    ret->data_float = value;
    ret->is_right = 0;
    ret->dimen = 0;
    return ret;
}

TypeNode* type_new_struct(int size) {
    TypeNode* ret = (TypeNode*)malloc(sizeof(TypeNode));
    ret->type = TYPE_STRUCT;
    ret->data_struct.size = size;
    ret->data_struct.types = (TypeNode**)malloc(sizeof(TypeNode) * size);
    ret->is_right = 0;
    ret->dimen = 0;
    return ret;
}

TypeNode* type_new_func(TypeNode* ret, TypeNode* args) {
    TypeNode* tmp = (TypeNode*)malloc(sizeof(TypeNode));
    tmp->type = TYPE_FUNC;
    tmp->data_func.ret = ret;
    tmp->data_func.args = args;
    ret->is_right = 0;
    return tmp;
}

int type_free(TypeNode* node) {
    if (!node)
        return -1;

    if (node->type == TYPE_STRUCT) {
        for (int i = 0; i < node->data_struct.size; i++)
            type_free(node->data_struct.types[i]);
    } else if (node->type == TYPE_FUNC) {
        type_free(node->data_func.ret);
        type_free(node->data_func.args);
    }

    FREE(node);
    return 0;
}

TypeNode* type_dup(TypeNode* type) {
    TypeNode* ret = (TypeNode*)malloc(sizeof(TypeNode));
    memcpy(ret, type, sizeof(TypeNode));
    return ret;
}

TypeNode* type_dup_right(TypeNode* type) {
    TypeNode* ret = type_dup(type);
    ret->is_right = 1;
    return ret;
}

TypeNode* type_dup_left(TypeNode* type) {
    TypeNode* ret = type_dup(type);
    ret->is_right = 0;
    return ret;
}