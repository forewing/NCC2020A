#include "symbol.h"
#include <stdio.h>
#include "hash.h"
#include "helper.h"
#include "syntax.tab.h"
#include "tree.h"

HashMap* symtab;
int age_now;

void symbol_error(int type, int lineno, const char* msg, const char* name) {
    fprintf(stderr, "Error type %d at Line %d: %s %s.\n", type, lineno, msg,
            name);
}

void state_Program(TreeNode* root);
void state_ExtDefList(TreeNode* root);
void state_ExtDef(TreeNode* root);
void state_ExtDecList(TreeNode* root, TypeNode* type);
TypeNode* state_Specifier(TreeNode* root);
TypeNode* state_StructSpecifier(TreeNode* root);
void state_OptTag(TreeNode* root);
void state_Tag(TreeNode* root);
TypeNode* state_VarDec(TreeNode* root, TypeNode* type);
TypeNode* state_FunDec(TreeNode* root, TypeNode* type);
void state_VarList(TreeNode* root, TypeNode** type_pos);
TypeNode* state_ParamDec(TreeNode* root);
void state_CompSt(TreeNode* root, TypeNode* func);
void state_StmtList(TreeNode* root);
void state_Stmt(TreeNode* root);
void state_DefList(TreeNode* root, TypeNode** type_pos);
void state_Def(TreeNode* root, TypeNode** type_pos);
void state_DecList(TreeNode* root, TypeNode* type, TypeNode** type_pos);
void state_Dec(TreeNode* root, TypeNode* type, TypeNode** type_pos);
TypeNode* state_Exp(TreeNode* root);
void state_Args(TreeNode* root);

void symtab_build() {
    symtab = hashmap_new();
    age_now = 0;
    state_Program(tree_root);
}

void state_Program(TreeNode* root) {
    if (!root)
        return;
    state_ExtDefList(root->children[0]);
}

void state_ExtDefList(TreeNode* root) {
    if (!root)
        return;
    if (root->size == 0)
        return;
    state_ExtDef(root->children[0]);
    state_ExtDefList(root->children[1]);
}

void state_ExtDef(TreeNode* root) {
    if (!root)
        return;
    TypeNode* type = state_Specifier(root->children[0]);
    if (root->children[1]->node_type == NODE_TERM) {
        // Specifier SEMI
        // Do nothing
        return;
    }
    if (root->children[1]->state_type == FunDec) {
        type = state_FunDec(root->children[1], type);
        TypeNode* pre = hashmap_value(symtab, type->name, age_now);
        if (pre) {
            if (!typeEqual(pre, type)) {
                symbol_error(19, root->lineno,
                             "inconsistent declaration of function",
                             type->name);
                hashmap_delete(symtab, type->name, age_now);
                hashmap_insert(symtab, type->name, age_now, type);
            } else {
                type = pre;
            }
        } else {
            hashmap_insert(symtab, type->name, age_now, type);
        }

        if (root->children[2]->node_type == NODE_NOTERM) {
            // Specifier FunDec CompSt
            state_CompSt(root->children[2], type);
            if (type->data_func.is_def) {
                symbol_error(4, root->lineno, "duplicated define of function",
                             type->name);
            }
            type->data_func.is_def = 1;
        }
    } else {
        // Specifier ExtDecList SEMI
        state_ExtDecList(root->children[1], type);
    }
}

void state_ExtDecList(TreeNode* root, TypeNode* type) {
    if (root->size == 1) {
        // VarDec
        TypeNode* node = state_VarDec(root->children[0], type_dup(type));
        if (hashmap_node(symtab, node->name, age_now)) {
            symbol_error(3, root->lineno, "redefined variable:", node->name);
            hashmap_delete(symtab, node->name, age_now);
        }
        hashmap_insert(symtab, node->name, age_now, node);
    } else {
        // VarDec COMMA ExtDecList
        state_ExtDecList(root->children[2], type);
    }
}

TypeNode* state_Specifier(TreeNode* root) {
    if (!root)
        return NULL;
    if (root->children[0]->node_type == NODE_TERM) {
        // TYPE
        if (root->children[0]->data_int == TYPENAME_INT)
            return type_new_int(0);
        return type_new_float(0);
    }
    return state_StructSpecifier(root->children[0]);
}

TypeNode* state_StructSpecifier(TreeNode* root) {
    if (root->size == 2) {
        // STRUCT Tag
        const char* name = root->children[1]->children[0]->data_str;
        TypeNode* type = hashmap_value(symtab, name, -1);
        if (!type) {
            symbol_error(17, root->lineno, "struct not defined:", name);
            type = type_new_invalid();
        }
        return type;
    } else {
        // STRUCT OptTag LC DefList RC
        int count = 0;
        TreeNode* defList = root->children[3];
        while (defList->size) {
            count += defList->children[0]->children[1]->data_int;
            defList = defList->children[1];
        }
        // printf("%d\n", count);

        age_now++;
        TypeNode* type = type_new_struct(count);
        // type->data_struct.is_type = 1;
        state_DefList(root->children[3], type->data_struct.types);
        hashmap_delete_age(symtab, age_now);
        age_now--;

        if (root->children[1]->size == 1) {
            hashmap_insert(symtab, root->children[1]->children[0]->data_str,
                           age_now, type);
        }
        // hashmap_insert(symtab, )
    }

    return NULL;
}

void state_OptTag(TreeNode* root) {
    // Do nothing
}

void state_Tag(TreeNode* root) {
    // Do nothing
}

TypeNode* state_VarDec(TreeNode* root, TypeNode* type) {
    if (root->size == 1) {
        // ID
        type->name = root->children[0]->data_str;
        type->dimen = 0;
        return type;
    } else {
        // VarDec LB INT RB
        state_VarDec(root->children[0], type);
        type->dimen++;
        return type;
    }
}

TypeNode* state_FunDec(TreeNode* root, TypeNode* type) {
    TypeNode* args;
    if (root->size == 4) {
        // ID LP VarList RP
        int count = root->children[2]->data_int;
        args = type_new_struct(count);
        // age_now++;
        state_VarList(root->children[2], args->data_struct.types);
        // hashmap_delete_age(symtab, age_now);
        // age_now--;
    } else {
        // ID LP RP
        args = type_new_struct(0);
    }

    TypeNode* tmp = type_new_func(type, args);
    tmp->name = root->children[0]->data_str;
    return tmp;
}

void state_VarList(TreeNode* root, TypeNode** type_pos) {
    TypeNode* arg = state_ParamDec(root->children[0]);
    // if (hashmap_node(symtab, arg->name, age_now)) {
    //     symbol_error();
    // }
    *type_pos = arg;
    if (root->size == 3) {
        // ParamDec COMMA VarList
        state_VarList(root->children[2], type_pos + 1);
    }
}

TypeNode* state_ParamDec(TreeNode* root) {
    // Specifier VarDec
    TypeNode* type = state_Specifier(root->children[0]);
    state_VarDec(root->children[1], type);
    return type;
}

void state_CompSt(TreeNode* root, TypeNode* func) {}

void state_StmtList(TreeNode* root) {}

void state_Stmt(TreeNode* root) {}

void state_DefList(TreeNode* root, TypeNode** type_pos) {
    if (!root || root->size == 0)
        return;
    state_Def(root->children[0], type_pos);
    if (type_pos) {
        state_DefList(root->children[1],
                      type_pos + root->children[0]->children[1]->data_int);
    } else {
        state_DefList(root->children[1], NULL);
    }
}

void state_Def(TreeNode* root, TypeNode** type_pos) {
    TypeNode* type = state_Specifier(root->children[0]);
    state_DecList(root->children[1], type, type_pos);
}

void state_DecList(TreeNode* root, TypeNode* type, TypeNode** type_pos) {
    state_Dec(root->children[0], type_dup(type), type_pos);
    if (root->size == 3) {
        if (type_pos)
            state_DecList(root->children[2], type, type_pos + 1);
        else
            state_DecList(root->children[2], type, NULL);
    }
}

void state_Dec(TreeNode* root, TypeNode* type, TypeNode** type_pos) {
    // VarDec
    TypeNode* node = state_VarDec(root->children[0], type);
    // printf("%d ", node->dimen);
    if (root->size == 3) {
        // VarDec ASSIGNOP Exp
        if (type_pos) {
            symbol_error(15, root->lineno, "init struct member:", node->name);
        } else if (node->dimen != 0) {
            symbol_error(7, root->lineno, "init array:", node->name);
        } else {
            TypeNode* exp = state_Exp(root->children[2]);
            if (!typeEqual(node, exp))
                symbol_error(5, root->lineno, "wrong type:", node->name);
        }
    }
    // Insert
    if (hashmap_node(symtab, node->name, age_now)) {
        if (type_pos) {
            symbol_error(15, root->lineno,
                         "duplicated struct member:", node->name);
        } else {
            symbol_error(3, root->lineno, "redefined variable:", node->name);
        }
        hashmap_delete(symtab, node->name, age_now);
    }
    hashmap_insert(symtab, node->name, age_now, node);
    if (type_pos)
        *type_pos = node;
}

TypeNode* state_Exp(TreeNode* root) {}

void state_Args(TreeNode* root) {}
