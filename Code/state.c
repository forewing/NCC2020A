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
#define rln root->lineno

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
    // All the return value should be dup before assign

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
            symbol_error(17, rln, "struct not defined: ", name);
            return &invalid_entity;
        }
        if (type->type != TYPE_STRUCT) {
            symbol_error(16, rln,
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
    SymNode* type = type_new_struct(size);
    symtab_push();
    state_DefList(rch3, type->data_struct.types);
    symtab_pop();

    const char* name = state_OptTag(rch1);
    if (name) {
        SymNode* pre = symtab_lookup_root(name);
        if (pre) {
            symbol_error(16, rln, "struct name duplicated:", name);
        } else {
            symtab_insert_root(name, type_dup_right(type));
        }
    }

    return type;
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

void state_DefList(TreeNode* root, SymNode** type_pos) {
    if (rsz == 0) {
        // Empty
        return;
    }
    // Def DefList
    state_Def(rch0, type_pos);
    if (type_pos) {
        state_DefList(rch1, type_pos + rch0->children[1]->data_int);
    } else {
        state_DefList(rch1, NULL);
    }
}

void state_Def(TreeNode* root, SymNode** type_pos) {
    // Specifier DecList SEMI
    SymNode* type = state_Specifier(rch0);
    state_DecList(rch1, type, type_pos);
}

void state_DecList(TreeNode* root, SymNode* type, SymNode** type_pos) {
    // Dec
    state_Dec(rch0, type_dup_left(type), type_pos);
    if (rsz == 3) {
        // Dec COMMA DecList
        if (type_pos)
            state_Declist(rch2, type, type_pos + 1);
        else
            state_DecList(rch2, type, NULL);
    }
}

void state_Dec(TreeNode* root, SymNode* type, SymNode** type_pos) {
    // type already duped
    // VarDec
    SymNode* node = state_VarDec(rch0, type);
    if (root->size == 3) {
        // VarDec ASSIGNOP Exp
        if (type_pos) {
            // In struct
            symbol_error(15, rln, "init struct member: ", node->name);
        } else if (node->type == TYPE_ARRAY) {
            // Is array
            symbol_error(5, rln, "init array: ", node->name);
        } else {
            SymNode* exp = state_Exp(rch2);
            if (!typeEqual(node, exp))
                symbol_error(5, rln, "conflict init type: ", node->name);
        }
    }
    // Insert
    if (symtab_lookup_last(node->name)) {
        // Conflict in current
        if (type_pos) {
            // In struct
            symbol_error(15, rln, "duplicated struct member: ", node->name);
        } else {
            // Variable
            symbol_error(3, rln, "redefined variable: ", node->name);
        }
    } else {
        SymNode* global = symtab_lookup_root(node->name);
        if (!type_pos && global && global->type == TYPE_STRUCT) {
            // Variable conflict with global struct
            symbol_error(
                3, rln,
                "variable name conflict with global struct name: ", node->name);
        }
        symtab_insert_last(node->name, type);
        if (type_pos)
            *type_pos = node;
    }
}

SymNode* state_Exp(TreeNode* root) {}
void state_Args(TreeNode* root, SymNode** type_pos) {}