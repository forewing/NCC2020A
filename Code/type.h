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
        } data_struct;
        struct {
            struct TypeNode* ret;
            struct TypeNode* args;
        } data_func;
    };
    const char* name;
    int dimen;
    int type;
    int is_right;
    int line;
} TypeNode;

int typeEqual(TypeNode* a, TypeNode* b);

TypeNode* type_new_invalid();
TypeNode* type_new_int(int value);
TypeNode* type_new_float(float value);
TypeNode* type_new_struct(int size);
TypeNode* type_new_func(TypeNode* ret, TypeNode* args);
int type_free(TypeNode* node);
TypeNode* type_dup(TypeNode* type);
TypeNode* type_dup_right(TypeNode* type);
TypeNode* type_dup_left(TypeNode* type);

#endif