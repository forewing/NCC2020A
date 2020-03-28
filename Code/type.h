#ifndef TYPE_H
#define TYPE_H

enum { TYPE_INT, TYPE_FLOAT, TYPE_STRUCT, TYPE_FUNC, TYPE_INVALID };

typedef struct TypeNode {
    union {
        int data_int;
        float data_float;
        struct {
            struct TypeNode** types;
            int size;
            int is_type;
        } data_struct;
        struct {
            struct TypeNode* ret;
            struct TypeNode* args;
            int is_def;
        } data_func;
    };
    int dimen;
    int type;
    const char* name;
} TypeNode;

int typeEqual(TypeNode* a, TypeNode* b);

TypeNode* type_new_invalid();
TypeNode* type_new_int(int value);
TypeNode* type_new_float(float value);
TypeNode* type_new_struct(int size);
TypeNode* type_new_func(TypeNode* ret, TypeNode* args);
int type_free(TypeNode* node);
TypeNode* type_dup(TypeNode* type);

#endif