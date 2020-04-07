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

typedef struct SymbolNode {
    union {
        int data_int;
        float data_float;
        struct {
            struct SymbolNode* next;
            int dimen;
            int size;
        } data_array;
        struct {
            struct SymbolNode** types;
            int size;
        } data_struct;
        struct {
            struct SymbolNode* ret;
            struct SymbolNode* args;
        } data_func;
    };
    const char* name;
    int offset;
    int type;
    int is_right;
    int line;
} SymbolNode;

int typeEqual(SymbolNode* a, SymbolNode* b);

SymbolNode* type_new();
SymbolNode* type_new_invalid();
SymbolNode* type_new_int(int value);
SymbolNode* type_new_float(float value);
SymbolNode* type_new_array(SymbolNode* next);
SymbolNode* type_new_struct(int size);
SymbolNode* type_new_func(SymbolNode* ret, SymbolNode* args);

int type_free(SymbolNode* node);
SymbolNode* type_dup(SymbolNode* type);
SymbolNode* type_dup_right(SymbolNode* type);
SymbolNode* type_dup_left(SymbolNode* type);

extern const SymbolNode int_entity;
extern const SymbolNode float_entity;
extern const SymbolNode void_entity;

#endif