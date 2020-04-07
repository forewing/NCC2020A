#include "state.h"

#include "symbol.h"
#include "symtab.h"
#include "tree.h"

void symbol_error(int type, int lineno, const char* msg, const char* name) {
    fprintf(stderr, "Error type %d at Line %d: %s%s.\n", type, lineno, msg,
            name);
}

#define CHILD(__ID__) root->children[__ID__]
#define rch0 CHILD(0)
#define rch1 CHILD(1)
#define rch2 CHILD(2)
#define rch3 CHILD(3)
#define rch4 CHILD(4)
#define rch5 CHILD(5)
#define rch6 CHILD(6)
#define rsz root->size

void state_Program(TreeNode* root);
void state_ExtDefList(TreeNode* root);
void state_ExtDef(TreeNode* root);
void state_ExtDecList(TreeNode* root, SymNode* type);
SymNode* state_Specifier(TreeNode* root);
SymNode* state_StructSpecifier(TreeNode* root);
const char* state_OptTag(TreeNode* root);
const char* state_Tag(TreeNode* root);
SymNode* state_VarDec(TreeNode* root, SymNode* type);
SymNode* state_FunDec(TreeNode* root, SymNode* type);
void state_VarList(TreeNode* root, SymNode** type_pos);
SymNode* state_ParamDec(TreeNode* root);
void state_CompSt(TreeNode* root, SymNode* func);
void state_StmtList(TreeNode* root, SymNode* func);
void state_Stmt(TreeNode* root, SymNode* func);
void state_DefList(TreeNode* root, SymNode** type_pos);
void state_Def(TreeNode* root, SymNode** type_pos);
void state_DecList(TreeNode* root, SymNode* type, SymNode** type_pos);
void state_Dec(TreeNode* root, SymNode* type, SymNode** type_pos);
SymNode* state_Exp(TreeNode* root);
void state_Args(TreeNode* root, SymNode** type_pos);

void symtab_build() {
    if (bug_number != 0)
        return;
    symtab = hashmap_new();
    symtab_root = symtab;
    state_Program(tree_root);
}

void state_Program(TreeNode* root) {
    state_ExtDefList(rch0);

    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* ptr = symtab->nodes[i];
        while (ptr) {
            SymNode* func = ptr->data;
            if (func->type == TYPE_FUNC && !func->is_right) {
                symbol_error(18, func->line,
                             "undefined function: ", func->name);
            }
            ptr = ptr->next;
        }
    }
}

void state_ExtDefList(TreeNode* root) {
    if (rsz == 2) {
        // ExtDef ExtDefList
        state_ExtDef(rch0);
        state_ExtDefList(rch1);
    }
    // else Empty
}

void state_ExtDef(TreeNode* root) {
    SymNode* spec = state_Specifier(rch0);
    if (rsz == 2) {
        // Specifier SEMI
        // Do nothing
        return;
    }
    if (rch1->state_type == STATE_ExtDecList) {
        // Specifier ExtDecList SEMI
        // state_ExtDecList(rch1, spec);
        return;
    }
    if (rch2->state_type == STATE_SEMI) {
        // Specifier FunDec SEMI
    } else {
        // Specifier FunDec CompSt
    }
}

void state_ExtDecList(TreeNode* root, SymNode* type) {}

SymNode* state_Specifier(TreeNode* root) {
    if (rch0->state_type == STATE_TYPE) {
        // TYPE
        switch (rch0->data_int) {
            case TYPENAME_INT:
                return &int_entity;
                break;
            case TYPENAME_FLOAT:
                return &float_entity;
                break;
            default:
                return &invalid_entity;
        }
    } else {
        // StructSpecifier
        return state_StructSpecifier(rch0);
    }
}

SymNode* state_StructSpecifier(TreeNode* root) {
    if (rsz == 2) {
        // STRUCT Tag
        const char* name = state_Tag(rch1);
        SymNode* type = symtab_lookup_root(name);
        if (!type) {
            symbol_error(17, root->lineno, "struct not defined: ", name);
            return &invalid_entity;
        }
        if (type->type != TYPE_STRUCT) {
            symbol_error(16, root->lineno,
                         "struct name duplicated with variable: ", name);
            return &invalid_entity;
        }
        return type;
    }
    // STRUCT OptTag LC DefList RC
    int size = 0;
    TreeNode* defList = rch3;
    while (defList->size) {
        size += defList->children[0]->children[1]->data_int;
        defList = defList->children[1];
    }
}
const char* state_OptTag(TreeNode* root) {
    if (rsz == 1) {
        // ID
        return rch0->data_str;
    }
    // Empty
    return NULL;
}

const char* state_Tag(TreeNode* root) {
    // ID
    return rch0->data_str;
}

SymNode* state_VarDec(TreeNode* root, SymNode* type) {}
SymNode* state_FunDec(TreeNode* root, SymNode* type) {}
void state_VarList(TreeNode* root, SymNode** type_pos) {}
SymNode* state_ParamDec(TreeNode* root) {}
void state_CompSt(TreeNode* root, SymNode* func) {}
void state_StmtList(TreeNode* root, SymNode* func) {}
void state_Stmt(TreeNode* root, SymNode* func) {}
void state_DefList(TreeNode* root, SymNode** type_pos) {}
void state_Def(TreeNode* root, SymNode** type_pos) {}
void state_DecList(TreeNode* root, SymNode* type, SymNode** type_pos) {}
void state_Dec(TreeNode* root, SymNode* type, SymNode** type_pos) {}
SymNode* state_Exp(TreeNode* root) {}
void state_Args(TreeNode* root, SymNode** type_pos) {}