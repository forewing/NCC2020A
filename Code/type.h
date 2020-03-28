#ifndef TYPE_H
#define TYPE_H

enum { TYPE_INT, TYPE_FLOAT, TYPE_ARRAY, TYPE_STRUCT, TYPE_FUNC };

typedef struct TypeNode {
    int type;
    union {
        int data_int;
        float data_float;
        struct {
            int size;
            struct TypeNode* type;
        } data_array;
        struct {
            int size;
            struct TypeNode** types;
        } data_struct;
        struct {
            struct TypeNode* ret;
            struct TypeNode* args;
        } data_func;
    };
} TypeNode;

int typeEqual(TypeNode* a, TypeNode* b);

TypeNode* type_new_int(int value);
TypeNode* type_new_float(float value);
TypeNode* type_new_array(int size, TypeNode* type);
TypeNode* type_new_struct(int size);
TypeNode* type_new_func(TypeNode* ret, TypeNode* args);
int type_free(TypeNode* node);

#endif