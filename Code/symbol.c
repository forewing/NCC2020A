#include "symbol.h"

#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "helper.h"
#include "syntax.tab.h"
#include "tree.h"

#define AGE_STRUCT 0

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
void state_StmtList(TreeNode* root, TypeNode* func);
void state_Stmt(TreeNode* root, TypeNode* func);
void state_DefList(TreeNode* root, TypeNode** type_pos);
void state_Def(TreeNode* root, TypeNode** type_pos);
void state_DecList(TreeNode* root, TypeNode* type, TypeNode** type_pos);
void state_Dec(TreeNode* root, TypeNode* type, TypeNode** type_pos);
TypeNode* state_Exp(TreeNode* root);
void state_Args(TreeNode* root, TypeNode** type_pos);

void symtab_build() {
    if (bug_number != 0)
        return;
    symtab = hashmap_new();
    age_now = 0;
    state_Program(tree_root);
}

void state_Program(TreeNode* root) {
    if (!root)
        return;
    state_ExtDefList(root->children[0]);

    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* ptr = symtab->nodes[i];
        while (ptr) {
            TypeNode* func = ptr->data;
            if (func->type == TYPE_FUNC && !func->is_right) {
                symbol_error(18, func->line, "undefined function:", func->name);
                // printf("%d\n", func->is_right);
            }
            ptr = ptr->next;
        }
    }
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
        // Specifier FunDec CompSt
        // Specifier FunDec SEMI
        type = state_FunDec(root->children[1], type);
        type->line = root->lineno;
        HashNode* pre = hashmap_node(symtab, type->name, age_now);
        if (pre) {
            type->is_right = pre->data->is_right;
            if (!typeEqual(pre->data, type)) {
                symbol_error(19, root->lineno,
                             "inconsistent declaration of function",
                             type->name);

                hashmap_delete(symtab, type->name, age_now);
                hashmap_insert(symtab, type->name, age_now, type);
            } else {
                pre->data = type;
            }
        } else {
            hashmap_insert(symtab, type->name, age_now, type);
        }

        if (root->children[2]->node_type == NODE_NOTERM) {
            // Specifier FunDec CompSt
            state_CompSt(root->children[2], type);
            if (type->is_right) {
                symbol_error(4, root->lineno,
                             "duplicated define of function:", type->name);
            }
            type->is_right = 1;
        }
    } else {
        // Specifier ExtDecList SEMI
        state_ExtDecList(root->children[1], type);
    }
}

void state_ExtDecList(TreeNode* root, TypeNode* type) {
    // VarDec
    TypeNode* node = state_VarDec(root->children[0], type_dup(type));
    if (hashmap_node(symtab, node->name, age_now)) {
        symbol_error(3, root->lineno, "redefined variable:", node->name);
        hashmap_delete(symtab, node->name, age_now);
    }
    TypeNode* pre = hashmap_value(symtab, node->name, AGE_STRUCT);
    if (pre && pre->type == TYPE_STRUCT) {
        symbol_error(3, root->lineno,
                     "variable name conflict with struct name:", node->name);
    }
    hashmap_insert(symtab, node->name, age_now, node);
    if (root->size == 3) {
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
    return type_dup_left(state_StructSpecifier(root->children[0]));
}

TypeNode* state_StructSpecifier(TreeNode* root) {
    if (root->size == 2) {
        // STRUCT Tag
        const char* name = root->children[1]->children[0]->data_str;
        TypeNode* type = hashmap_value(symtab, name, AGE_STRUCT);
        if (!type) {
            symbol_error(17, root->lineno, "struct not defined:", name);
            type = type_new_invalid();
        } else if (type->type != TYPE_STRUCT) {
            symbol_error(16, root->lineno,
                         "struct name duplicated with variable:", name);
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

        // Register to global first
        TypeNode* type = type_new_struct(count);

        age_now++;
        state_DefList(root->children[3], type->data_struct.types);
        hashmap_delete_age(symtab, age_now);
        age_now--;

        if (root->children[1]->size == 1) {
            const char* name = root->children[1]->children[0]->data_str;
            TypeNode* pre = hashmap_value(symtab, name, AGE_STRUCT);
            if (pre) {
                symbol_error(16, root->lineno, "struct name duplicated:", name);
                hashmap_delete(symtab, name, AGE_STRUCT);
            }
            hashmap_insert(symtab, name, AGE_STRUCT, type);
        }

        return type;
    }
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
        age_now++;
        state_VarList(root->children[2], args->data_struct.types);
        hashmap_delete_age(symtab, age_now);
        age_now--;
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
    if (hashmap_node(symtab, arg->name, age_now)) {
        symbol_error(3, root->lineno, "parameter name redefined:", arg->name);
    } else {
        hashmap_insert(symtab, arg->name, age_now, arg);
    }
    TypeNode* global = hashmap_value(symtab, arg->name, AGE_STRUCT);
    if (global && global->type == TYPE_STRUCT) {
        symbol_error(
            3, root->lineno,
            "parameter name duplicated with global struct:", arg->name);
    }
    *type_pos = type_dup(arg);
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

void state_CompSt(TreeNode* root, TypeNode* func) {
    age_now++;

    // Spray args
    if (func) {
        TypeNode* args = func->data_func.args;
        if (args) {
            for (int i = 0; i < args->data_struct.size; i++) {
                const char* name = args->data_struct.types[i]->name;
                TypeNode* pre = hashmap_value(symtab, name, AGE_STRUCT);
                if (pre && pre->type == TYPE_STRUCT) {
                    symbol_error(
                        3, root->lineno,
                        "parameter name duplicated with global struct:", name);
                }
                // printf("insert %s\n", args->data_struct.types[i]->name);
                hashmap_insert(symtab, name, age_now,
                               args->data_struct.types[i]);
            }
        }
    }

    state_DefList(root->children[1], NULL);

    // hashmap_print(symtab);

    if (func)
        state_StmtList(root->children[2], func);
    else
        state_StmtList(root->children[2], NULL);

    hashmap_delete_age(symtab, age_now);
    age_now--;
}

void state_StmtList(TreeNode* root, TypeNode* func) {
    if (root->size == 2) {
        // Stmt StmtList
        state_Stmt(root->children[0], func);
        state_StmtList(root->children[1], func);
    }
}

void state_Stmt(TreeNode* root, TypeNode* func) {
    if (root->size == 2) {
        // 2 Exp SEMI
        state_Exp(root->children[0]);
    } else if (root->size == 1) {
        // 1 CompSt
        state_CompSt(root->children[0], func);
    } else if (root->children[0]->state_type == RETURN) {
        // 3 RETURN Exp SEMI
        TypeNode* exp_ret = state_Exp(root->children[1]);
        if (!typeEqual(exp_ret, func->data_func.ret)) {
            symbol_error(8, root->lineno, "return type mismatch", "");
        }
    } else {
        // 5 IF LP Exp RP Stmt
        // 5 WHILE LP Exp RP Stmt
        TypeNode* exp = state_Exp(root->children[2]);
        if (!typeEqual(exp, type_new_int(0))) {
            symbol_error(7, root->lineno, "condition must be int", "");
        }
        state_Stmt(root->children[4], func);
        if (root->size == 7) {
            // 7 IF LP Exp RP Stmt ELSE Stmt
            state_Stmt(root->children[6], func);
        }
    }
    return;
}

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
        // Dec COMMA DecList
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
            symbol_error(5, root->lineno, "init array:", node->name);
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
    TypeNode* global = hashmap_value(symtab, node->name, AGE_STRUCT);
    if (!type_pos && global && global->type == TYPE_STRUCT) {
        symbol_error(
            3, root->lineno,
            "variable name conflict with global struct name:", node->name);
    }
    hashmap_insert(symtab, node->name, age_now, node);
    if (type_pos)
        *type_pos = node;
}

TypeNode* state_Exp(TreeNode* root) {
    TreeNode** children = root->children;

    TypeNode* id = NULL;
    if (children[0]->state_type == ID && children[0]->node_type == NODE_TERM) {
        id = hashmap_value(symtab, children[0]->data_str, -1);
        if (!id) {
            if (root->size == 1) {
                symbol_error(1, root->lineno,
                             "undefined variable:", children[0]->data_str);
            } else {
                symbol_error(2, root->lineno,
                             "undefined function:", children[0]->data_str);
            }
            return type_new_invalid();
        }
    }
    if (root->size == 1) {
        // ID, INT, FLOAT
        if (children[0]->state_type == ID) {
            return id;
        } else if (children[0]->state_type == INT) {
            return type_dup_right(type_new_int(children[0]->data_int));
        } else {
            return type_dup_right(type_new_float(children[0]->data_float));
        }
        return type_new_invalid();
    } else if (children[1]->state_type == LP &&
               children[1]->node_type == NODE_TERM) {
        if (id->type == TYPE_INVALID) {
            return id;
        }
        // printf("%s %d\n", id->name, id->type);
        if (id->type != TYPE_FUNC) {
            symbol_error(11, root->lineno,
                         "variable is not callable:", id->name);
            return type_new_invalid();
        } else if (root->size == 3) {
            // ID LP RP
            if (!typeEqual(type_new_struct(0), id->data_func.args)) {
                symbol_error(9, root->lineno, "arguments mismatch:", id->name);
                return type_new_invalid();
            }
            return type_dup_right(id->data_func.ret);
        } else {
            // ID LP Args RP
            if (children[2]->data_int == id->data_func.args->data_struct.size) {
                TypeNode* args = type_new_struct((int)children[2]->data_int);
                state_Args(children[2], args->data_struct.types);
                if (typeEqual(args, id->data_func.args)) {
                    // Only success here
                    return type_dup_right(id->data_func.ret);
                }
            }
            symbol_error(9, root->lineno, "arguments mismatch:", id->name);
            return type_new_invalid();
        }
    } else if (root->size == 2) {
        TypeNode* exp = state_Exp(children[1]);
        if (children[0]->state_type == MINUS) {
            // MINUS Exp
            if (!typeEqual(exp, type_new_int(1)) &&
                !typeEqual(exp, type_new_float(1))) {
                symbol_error(7, root->lineno,
                             "can't do arithmetic operation on types except "
                             "int and float",
                             "");
                return type_new_invalid();
            }
        } else {
            // NOT Exp
            if (!typeEqual(exp, type_new_int(1))) {
                symbol_error(7, root->lineno,
                             "can't do logic operation on types except int",
                             "");
                return type_new_invalid();
            }
        }
        return type_dup_right(exp);
    } else if (children[1]->node_type == NODE_NOTERM) {
        // LP Exp RP
        return state_Exp(root->children[1]);
    } else if (root->size == 3) {
        TypeNode* left = state_Exp(children[0]);
        if (!left || left->type == TYPE_INVALID) {
            return left;
        }
        TypeNode* right = NULL;
        if (children[2]->node_type == NODE_NOTERM) {
            right = state_Exp(children[2]);
        }
        switch (children[1]->state_type) {
            case ASSIGNOP:
                // Exp ASSIGNOP Exp
                if (!typeEqual(left, right)) {
                    symbol_error(5, root->lineno,
                                 "assign between different types", "");
                    return type_new_invalid();
                }
                if (left->is_right) {
                    symbol_error(6, root->lineno,
                                 "right value can't be assigned", "");
                    return type_new_invalid();
                }
                return left;
                break;
            case AND:
            case OR:
                // Exp AND      Exp
                // Exp OR       Exp
                if (!typeEqual(left, type_new_int(1)) ||
                    !typeEqual(right, type_new_int(1))) {
                    symbol_error(7, root->lineno,
                                 "can't do logic operation on types except int",
                                 "");
                    return type_new_invalid();
                }
                return type_dup_right(left);
                break;
            case RELOP:
            case PLUS:
            case MINUS:
            case STAR:
            case DIV:
                // Exp RELOP    Exp
                // Exp PLUS     Exp
                // Exp MINUS    Exp
                // Exp STAR     Exp
                // Exp DIV      Exp
                if (!typeEqual(left, right)) {
                    symbol_error(7, root->lineno,
                                 "cant't do operation between different types",
                                 "");
                    return type_new_invalid();
                }
                if (!typeEqual(left, type_new_float(1.1)) &&
                    !typeEqual(left, type_new_int(1))) {
                    symbol_error(7, root->lineno,
                                 "cant't do arithmetic operation on types "
                                 "except int and float",
                                 "");
                    return type_new_invalid();
                }
                return type_dup_right(left);
                break;
            case DOT:
                // Exp DOT ID
                if (left->type != TYPE_STRUCT) {
                    symbol_error(13, root->lineno, "variable not a struct", "");
                    return type_new_invalid();
                }
                TypeNode* ret = NULL;
                for (int i = 0; i < left->data_struct.size; i++) {
                    if (!strcmp(left->data_struct.types[i]->name,
                                children[2]->data_str)) {
                        ret = left->data_struct.types[i];
                        break;
                    }
                }
                if (!ret) {
                    symbol_error(14, root->lineno,
                                 "unknown field:", children[2]->data_str);
                    return type_new_invalid();
                }
                return ret;
                break;
        }

    } else {
        // Exp LB Exp RB
        TypeNode* elem = state_Exp(children[0]);
        if (elem->dimen <= 0) {
            symbol_error(10, root->lineno, "indexing non-array variable", "");
            return type_new_invalid();
        }
        TypeNode* index = state_Exp(children[2]);
        if (!typeEqual(index, type_new_int(1))) {
            symbol_error(12, root->lineno, "index is not int", "");
            return type_new_invalid();
        }
        TypeNode* ret = type_dup(elem);
        ret->dimen--;
        return ret;
    }
    return type_new_invalid();
    // return NULL;
}

void state_Args(TreeNode* root, TypeNode** type_pos) {
    // Exp
    *type_pos = state_Exp(root->children[0]);
    if (root->size == 3) {
        // Exp COMMA Args
        state_Args(root->children[2], type_pos + 1);
    }
}
