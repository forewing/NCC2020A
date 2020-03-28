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
void state_ExtDecList(TreeNode* root);
TypeNode* state_Specifier(TreeNode* root);
TypeNode* state_StructSpecifier(TreeNode* root);
void state_OptTag(TreeNode* root);
void state_Tag(TreeNode* root);
TypeNode* state_VarDec(TreeNode* root, TypeNode* type);
TypeNode* state_FunDec(TreeNode* root, TypeNode* type);
void state_VarList(TreeNode* root);
void state_ParamDec(TreeNode* root);
void state_CompSt(TreeNode* root);
void state_StmtList(TreeNode* root);
void state_Stmt(TreeNode* root);
void state_DefList(TreeNode* root, TypeNode** type_pos);
void state_Def(TreeNode* root, TypeNode** type_pos);
void state_DecList(TreeNode* root, TypeNode* type, TypeNode** type_pos);
void state_Dec(TreeNode* root, TypeNode* type, TypeNode** type_pos);
void state_Exp(TreeNode* root);
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
        if (root->children[2]->node_type == NODE_NOTERM) {
            // Specifier FunDec CompSt
            state_CompSt(root->children[2]);
            type->data_func.is_def = 1;
        }
    } else {
        // Specifier ExtDecList SEMI
    }
}

void state_ExtDecList(TreeNode* root) {}

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
    // TODO
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
        type->data_struct.is_type = 1;
        state_DefList(root->children[3], type->data_struct.types);
        age_now--;

        if (root->children[1]->size == 1) {
            hashmap_insert(symtab, root->children[1]->children[0]->data_str,
                           age_now, type);
        }
        // hashmap_insert(symtab, )
    }

    return NULL;
}

void state_OptTag(TreeNode* root) {}

void state_Tag(TreeNode* root) {}

TypeNode* state_VarDec(TreeNode* root, TypeNode* type) {
    // TODO
}

TypeNode* state_FunDec(TreeNode* root, TypeNode* type) {}

void state_VarList(TreeNode* root) {}

void state_ParamDec(TreeNode* root) {}

void state_CompSt(TreeNode* root) {}

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
    state_Dec(root->children[0], type, type_pos);
    if (root->size == 3)
        state_DecList(root->children[2], type, type_pos + 1);
}

void state_Dec(TreeNode* root, TypeNode* type, TypeNode** type_pos) {
    // TODO
    // VarDec
    TypeNode* node = state_VarDec(root->children[0], type);
    if (root->size == 3) {
        // VarDec ASSIGNOP Exp
        if (type_pos)
            symbol_error(15, root->lineno, "init struct member", "");
    }
}

void state_Exp(TreeNode* root) {}

void state_Args(TreeNode* root) {}