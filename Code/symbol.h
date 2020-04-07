#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include <string.h>

#include "helper.h"

enum SYMBOL_TYPE {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_FUNC,
    TYPE_INVALID
};

typedef struct SymNode {
    union {
        int data_int;
        float data_float;
        struct {
            struct SymNode* next;
            int dimen;
            int size;
        } data_array;
        struct {
            struct SymNode** types;
            int size;
        } data_struct;
        struct {
            struct SymNode* ret;
            struct SymNode* args;
        } data_func;
    };
    const char* name;
    int offset;
    int type;
    int is_right;
    int line;
} SymNode;

int typeEqual(SymNode* a, SymNode* b);

SymNode* type_new();
SymNode* type_new_invalid();
SymNode* type_new_int(int value);
SymNode* type_new_float(float value);
SymNode* type_new_array(SymNode* next);
SymNode* type_new_struct(int size);
SymNode* type_new_func(SymNode* ret, SymNode* args);

int type_free(SymNode* node);
SymNode* type_dup(SymNode* type);
SymNode* type_dup_right(SymNode* type);
SymNode* type_dup_left(SymNode* type);

extern const SymNode int_entity;
extern const SymNode float_entity;
extern const SymNode void_entity;
extern const SymNode invalid_entity;

#endif