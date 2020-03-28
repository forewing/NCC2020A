#include "type.h"
#include <stdlib.h>
#include "helper.h"

int typeEqual(TypeNode* a, TypeNode* b) {
    if (!a || !b)
        return 0;
    if (a->type != b->type)
        return 0;

    if (a->type == TYPE_ARRAY) {
        if (a->data_array.size == b->data_array.size &&
            typeEqual(a->data_array.type, b->data_array.type))
            return 1;
        return 0;
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

    if (a->type == b->type)
        return 1;

    return 0;
}

TypeNode* type_new_int(int value) {
    TypeNode* ret = (TypeNode*)malloc(sizeof(TypeNode));
    ret->type = TYPE_INT;
    ret->data_int = value;
    return ret;
}

TypeNode* type_new_float(float value) {
    TypeNode* ret = (TypeNode*)malloc(sizeof(TypeNode));
    ret->type = TYPE_FLOAT;
    ret->data_float = value;
    return ret;
}

TypeNode* type_new_array(int size, TypeNode* type) {
    TypeNode* ret = (TypeNode*)malloc(sizeof(TypeNode));
    ret->type = TYPE_ARRAY;
    ret->data_array.size = size;
    ret->data_array.type = type;
    return ret;
}

TypeNode* type_new_struct(int size) {
    TypeNode* ret = (TypeNode*)malloc(sizeof(TypeNode));
    ret->type = TYPE_STRUCT;
    ret->data_struct.size = size;
    ret->data_struct.types = (TypeNode**)malloc(sizeof(TypeNode) * size);
    return ret;
}

TypeNode* type_new_func(TypeNode* ret, TypeNode* args) {
    TypeNode* tmp = (TypeNode*)malloc(sizeof(TypeNode));
    tmp->type = TYPE_FUNC;
    tmp->data_func.ret = ret;
    tmp->data_func.args = args;
    return tmp;
}

int type_free(TypeNode* node) {
    if (!node)
        return -1;

    if (node->type == TYPE_ARRAY) {
        type_free(node->data_array.type);
    } else if (node->type == TYPE_STRUCT) {
        for (int i = 0; i < node->data_struct.size; i++)
            type_free(node->data_struct.types[i]);
    } else if (node->type == TYPE_FUNC) {
        type_free(node->data_func.ret);
        type_free(node->data_func.args);
    }

    FREE(node);
    return 0;
}