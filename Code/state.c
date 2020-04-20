#include "state.h"

#include "helper.h"
#include "symbol.h"
#include "symtab.h"
#include "tree.h"

void symbol_error(int type, int lineno, const char* msg, const char* name) {
    bug_number++;
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
void state_CompSt(TreeNode* root, SymNode* ret, SymNode* args);
void state_StmtList(TreeNode* root, SymNode* ret);
void state_Stmt(TreeNode* root, SymNode* ret);
void state_DefList(TreeNode* root, SymNode** type_pos);
void state_Def(TreeNode* root, SymNode** type_pos);
void state_DecList(TreeNode* root, SymNode* type, SymNode** type_pos);
void state_Dec(TreeNode* root, SymNode* type, SymNode** type_pos);
SymNode* state_Exp(TreeNode* root);
void state_Args(TreeNode* root, SymNode** type_pos);

void symtab_build() {
    if (bug_number != 0)
        return;
    struct_table = hashmap_new();
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
        state_ExtDecList(rch1, spec);
        return;
    }

    SymNode* func = state_FunDec(rch1, spec);
    func->line = rch1->lineno;
    HashNode* pre = symtab_place_now(func->name);
    if (pre) {
        func->is_right = pre->data->is_right;
        if (!typeEqual(func, pre->data)) {
            symbol_error(19, rln,
                         "inconsistent declaration of function: ", func->name);
        }
        pre->data = func;
    } else {
        symtab_insert_now(func->name, func);
    }
    if (rch2->state_type == STATE_CompSt) {
        // Specifier FunDec CompSt
        state_CompSt(rch2, func->data_func.ret, func->data_func.args);
        if (func->is_right) {
            symbol_error(4, rln, "duplicated define of function: ", func->name);
        }
        func->is_right = 1;
    }
}

void state_ExtDecList(TreeNode* root, SymNode* type) {
    // VarDec

    SymNode* node = state_VarDec(rch0, type_dup_left(type));

    if (symtab_lookup_now(node->name)) {
        // Conflict
        symbol_error(3, rln, "redefined variable: ", node->name);
    } else {
        SymNode* global = symtab_lookup(struct_table, node->name);
        if (global) {
            // Conflict with global struct name
            symbol_error(3, rln, "variable name conflict with struct name: ",
                         node->name);
        }
        symtab_insert_now(node->name, node);
    }

    if (rsz == 3) {
        // VarDec COMMA ExtDecList
        state_ExtDecList(rch2, type);
    }
}

SymNode* state_Specifier(TreeNode* root) {
    // All the return value should be dup before assign

    if (rch0->state_type == STATE_TYPE) {
        // TYPE
        switch (rch0->data_int) {
            case TYPENAME_INT:
                return type_new_int(0);
                break;
            case TYPENAME_FLOAT:
                return type_new_float(0.0);
                break;
            default:
                return type_new_invalid();
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
        SymNode* type = symtab_lookup(struct_table, name);
        if (!type) {
            symbol_error(17, rln, "struct not defined: ", name);
            return type_new_invalid();
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
        SymNode* pre = symtab_lookup(struct_table, name);
        if (pre) {
            symbol_error(16, rln, "struct name duplicated: ", name);
        } else {
            SymNode* entity = type_dup_right(type);
            symtab_insert(struct_table, name, entity);
        }
        if (symtab_lookup_root(name)) {
            symbol_error(16, rln, "struct name duplicated: ", name);
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

SymNode* state_VarDec(TreeNode* root, SymNode* type) {
    // type should be dup already
    if (rsz == 1) {
        // ID
        type->name = rch0->data_str;
        return type;
    } else {
        // VarDec LB INT RB
        SymNode* pre = state_VarDec(rch0, type);
        SymNode* ret = type_new_array(pre);
        ret->data_array.size = rch2->data_int;
        ret->name = pre->name;
        if (pre->type == TYPE_ARRAY) {
            ret->data_array.dimen = pre->data_array.dimen + 1;
        } else {
            ret->data_array.dimen = 1;
        }
        return ret;
    }
}

SymNode* state_FunDec(TreeNode* root, SymNode* type) {
    SymNode* args;
    if (root->size == 4) {
        // ID LP VarList RP
        int size = rch2->data_int;
        args = type_new_struct(size);
        symtab_push();
        state_VarList(rch2, args->data_struct.types);
        symtab_pop();
    } else {
        // ID LP RP
        args = type_dup_left(&void_entity);
    }

    SymNode* ret = type_new_func(type, args);
    ret->name = rch0->data_str;
    return ret;
}

void state_VarList(TreeNode* root, SymNode** type_pos) {
    // ParamDec
    SymNode* arg = state_ParamDec(rch0);
    if (symtab_lookup_now(arg->name)) {
        symbol_error(3, rln, "parameter name redefined: ", arg->name);
    } else {
        symtab_insert_now(arg->name, arg);
    }

    SymNode* global = symtab_lookup(struct_table, arg->name);
    if (global) {
        symbol_error(3, rln, "parameter name duplicated with global struct: ",
                     arg->name);
    }

    *type_pos = arg;

    if (rsz == 3) {
        // ParamDec COMMA VarList
        state_VarList(rch2, type_pos + 1);
    }
}

SymNode* state_ParamDec(TreeNode* root) {
    // Specifier VarDec
    SymNode* type = type_dup_left(state_Specifier(rch0));
    return state_VarDec(rch1, type);
}

void state_CompSt(TreeNode* root, SymNode* ret, SymNode* args) {
    symtab_push();

    if (args) {
        // Spray args
        for (int i = 0; i < args->data_struct.size; i++) {
            const char* name = args->data_struct.types[i]->name;
            SymNode* pre = symtab_lookup(struct_table, name);
            if (pre) {
                symbol_error(
                    3, rln,
                    "parameter name duplicated with global struct: ", name);
            }
            symtab_insert_now(name, args->data_struct.types[i]);
        }
    }

    // LC DefList StmtList RC
    state_DefList(rch1, NULL);
    state_StmtList(rch2, ret);

    symtab_pop();
}

void state_StmtList(TreeNode* root, SymNode* ret) {
    if (rsz == 2) {
        // Stmt StmtList
        state_Stmt(rch0, ret);
        state_StmtList(rch1, ret);
    }
    // else Empty
}

void state_Stmt(TreeNode* root, SymNode* ret) {
    if (rsz == 1) {
        // CompSt
        state_CompSt(rch0, ret, NULL);
    } else if (rsz == 2) {
        // Exp SEMI
        state_Exp(rch0);
    } else if (rsz == 3) {
        // RETURN Exp SEMI
        SymNode* exp = state_Exp(rch1);
        if (!typeEqual(exp, ret)) {
            symbol_error(8, rln, "return type mismatch", "");
        }
    } else {
        // IF    LP Exp RP Stmt
        // IF    LP Exp RP Stmt ELSE Stmt
        // WHILE LP Exp RP Stmt
        SymNode* exp = state_Exp(rch2);
        if (!typeEqual(exp, &int_entity)) {
            symbol_error(7, root->lineno, "condition must be int", "");
        }
        state_Stmt(rch4, ret);
        if (root->size == 7) {
            // IF    LP Exp RP Stmt ELSE Stmt
            state_Stmt(rch6, ret);
        }
    }
}

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
            state_DecList(rch2, type, type_pos + 1);
        else
            state_DecList(rch2, type, NULL);
    }
}

void state_Dec(TreeNode* root, SymNode* type, SymNode** type_pos) {
    // type already duped
    SymNode* node = state_VarDec(rch0, type);

    // Insert
    if (symtab_lookup_now(node->name)) {
        // Conflict in current
        if (type_pos) {
            // In struct
            symbol_error(15, rln, "duplicated struct member: ", node->name);
        } else {
            // Variable
            symbol_error(3, rln, "redefined variable: ", node->name);
        }
    } else {
        SymNode* global = symtab_lookup(struct_table, node->name);
        if (!type_pos && global && global->type == TYPE_STRUCT) {
            // Variable conflict with global struct
            symbol_error(
                3, rln,
                "variable name conflict with global struct name: ", node->name);
        }
        symtab_insert_now(node->name, node);
    }
    if (type_pos) {
        *type_pos = node;
    }

    // VarDec
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
}

SymNode* state_Exp(TreeNode* root) {
    if (rch0->state_type == STATE_INT) {
        // INT
        SymNode* type = type_new_int(rch0->data_int);
        type->is_right = 1;
        return type;
    } else if (rch0->state_type == STATE_FLOAT) {
        // FLOAT
        SymNode* type = type_new_float(rch0->data_float);
        type->is_right = 1;
        return type;
    }

    if (rch0->state_type == STATE_ID) {
        SymNode* id = symtab_lookup_all(rch0->data_str);
        if (rsz == 1) {
            // ID
            if (!id) {
                symbol_error(1, rln, "undefined variable: ", rch0->data_str);
                return type_new_invalid();
            }
            return id;
        }
        // ID LP Args RP
        // ID LP RP
        if (!id) {
            symbol_error(2, rln, "undefined function: ", rch0->data_str);
            return type_new_invalid();
        }
        if (id->type == TYPE_INVALID) {
            return id;
        }
        if (id->type != TYPE_FUNC) {
            symbol_error(11, rln, "variable is not callable: ", id->name);
            return type_new_invalid();
        }
        if (rsz == 3) {
            // ID LP RP
            if (!typeEqual(&void_entity, id->data_func.args)) {
                symbol_error(9, rln, "arguments mismatch: ", id->name);
            }
            return type_dup_right(id->data_func.ret);
        } else {
            // ID LP Args RP
            if (rch2->data_int == id->data_func.args->data_struct.size) {
                SymNode* args = type_new_struct(rch2->data_int);
                state_Args(rch2, args->data_struct.types);
                if (typeEqual(args, id->data_func.args)) {
                    // Only success here
                    return type_dup_right(id->data_func.ret);
                }
            }
            // Fail here
            symbol_error(9, rln, "arguments mismatch: ", id->name);
            return type_new_invalid();
        }
    }

    if (rch0->state_type == STATE_LP) {
        // LP Exp RP
        return state_Exp(rch1);
    }

    // if (rsz == )
    SymNode* exp1 = NULL;
    if (rch0->state_type == STATE_Exp) {
        // others
        exp1 = state_Exp(rch0);
    } else {
        // MINUS Exp
        // NOT Exp
        exp1 = state_Exp(rch1);
    }

    SymNode* exp2 = NULL;
    if (rsz >= 3 && rch2->state_type == STATE_Exp) {
        exp2 = state_Exp(rch2);
    }

    if (exp1->type == TYPE_INVALID) {
        return type_new_invalid();
    }
    if (exp2 && exp2->type == TYPE_INVALID) {
        return type_new_invalid();
    }

    if (rsz == 2) {
        if (rch0->state_type == STATE_MINUS) {
            // MINUS Exp
            if (!typeEqual(exp1, &int_entity) &&
                !typeEqual(exp1, &float_entity)) {
                symbol_error(7, rln,
                             "can't do arithmetic operation on types except "
                             "int and float",
                             "");
                return type_new_invalid();
            }
        } else {
            // NOT Exp
            if (!typeEqual(exp1, &int_entity)) {
                symbol_error(
                    7, rln, "can't do logic operation on types except int", "");
                return type_new_invalid();
            }
        }
        return type_dup_right(exp1);
    }

    if (rch1->state_type == STATE_DOT) {
        // Exp DOT ID
        if (exp1->type != TYPE_STRUCT) {
            symbol_error(13, rln, "variable not a struct", "");
            return type_new_invalid();
        }
        SymNode* ret = NULL;
        for (int i = 0; i < exp1->data_struct.size; i++) {
            if (!strcmp(exp1->data_struct.types[i]->name, rch2->data_str)) {
                ret = exp1->data_struct.types[i];
                break;
            }
        }
        if (!ret) {
            symbol_error(14, rln, "unknown field: ", rch2->data_str);
            return type_new_invalid();
        }
        return ret;
    } else if (rsz == 4) {
        // Exp LB Exp RB
        if (exp1->type != TYPE_ARRAY) {
            symbol_error(10, rln, "indexing non-array variable", "");
            return type_new_invalid();
        }
        if (!typeEqual(exp2, &int_entity)) {
            symbol_error(12, rln, "index is not int", "");
        }
        return exp1->data_array.next;
    }

    switch (rch1->state_type) {
        case STATE_ASSIGNOP:
            // Exp ASSIGNOP Exp
            if (!typeEqual(exp1, exp2)) {
                symbol_error(5, rln, "assign between different types", "");
                return type_new_invalid();
            }
            if (exp1->is_right) {
                symbol_error(6, rln, "right value can't be assigned", "");
                return type_new_invalid();
            }
            return exp1;
            break;
        case STATE_AND:
        case STATE_OR:
            // Exp AND Exp
            // Exp OR Exp
            if (!typeEqual(exp1, &int_entity) ||
                !typeEqual(exp2, &int_entity)) {
                symbol_error(
                    7, rln, "can't do logic operation on types except int", "");
                return type_new_invalid();
            }
            return type_dup_right(exp1);
            break;
        case STATE_RELOP:
        case STATE_PLUS:
        case STATE_MINUS:
        case STATE_STAR:
        case STATE_DIV:
            // Exp RELOP Exp
            // Exp PLUS Exp
            // Exp MINUS Exp
            // Exp STAR Exp
            // Exp DIV Exp
            if (!typeEqual(exp1, exp2)) {
                symbol_error(7, rln,
                             "cant't do operation between different types", "");
                return type_new_invalid();
            }
            if (!typeEqual(exp1, &int_entity) &&
                !typeEqual(exp1, &float_entity)) {
                symbol_error(7, rln,
                             "cant't do arithmetic operation on types "
                             "except int and float",
                             "");
                return type_new_invalid();
            }
            return type_dup_right(exp1);
            break;
    }
    return type_new_invalid();
}

void state_Args(TreeNode* root, SymNode** type_pos) {
    // Exp
    *type_pos = state_Exp(rch0);
    if (rsz == 3) {
        // Exp COMMA Args
        state_Args(rch2, type_pos + 1);
    }
}