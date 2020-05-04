#include "state.h"

#include "helper.h"
#include "ir.h"
#include "symbol.h"
#include "symtab.h"
#include "tree.h"

// void symbol_error(int type, int lineno, const char* msg, const char* name) {
//     bug_number++;
//     fprintf(stderr, "Error type %d at Line %d: %s%s.\n", type, lineno, msg,
//             name);
// }

typedef struct ExpRet_t {
    SymNode* node;
    int addr;
} ExpRet_t;
ExpRet_t ExpRet_val(SymNode* node) {
    ExpRet_t ret = {.node = node, .addr = -1};
    return ret;
}
ExpRet_t ExpRet_addr(SymNode* node, int addr_tmp) {
    ExpRet_t ret = {.node = node, .addr = addr_tmp};
    return ret;
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

void CODE_INSERT(int type, int data, IrOprand* x, IrOprand* y, IrOprand* z) {
    IrCode_insert(ircode_list, IrCode_new(type, data, x, y, z));
}

void struct_size_calc(SymNode* elem) {
    for (int i = 0; i < elem->data_struct.size; i++) {
        elem->size += elem->data_struct.types[i]->size;
    }
}

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
ExpRet_t state_Exp(TreeNode* root, int target);
void state_Cond(TreeNode* root, int label_true, int label_false);
void state_Args(TreeNode* root, SymNode** type_pos, int* tmp_pos);

void symtab_build() {
    if (bug_number != 0)
        return;
    struct_table = hashmap_new();
    symtab = hashmap_new();
    symtab_root = symtab;

    ircode_list = IrCode_new(CODE_NOP, 0, NULL, NULL, NULL);

    state_Program(tree_root);
}

void state_Program(TreeNode* root) {
    SymNode* func_write = type_new_func(type_dup_right(&int_entity), type_new_struct(1));
    func_write->name = "write";
    func_write->is_right = 1;
    func_write->data_func.args->size = 4;
    func_write->data_func.args->data_struct.types[0] = type_dup_right(&int_entity);
    symtab_insert_root("write", func_write);

    SymNode* func_read = type_new_func(type_dup_right(&int_entity), type_dup_right(&void_entity));
    func_read->name = "read";
    func_read->is_right = 1;
    symtab_insert_root("read", func_read);

    state_ExtDefList(rch0);
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
        // Global variable not allowed
        bug_number++;
        return;

        // Specifier ExtDecList SEMI
        // state_ExtDecList(rch1, spec);
        // return;
    }

    SymNode* func = state_FunDec(rch1, spec);
    func->line = rch1->lineno;
    symtab_insert_now(func->name, func);

    if (rch2->state_type == STATE_CompSt) {
        // Specifier FunDec CompSt

        CODE_INSERT(CODE_FUNC, 0, IrOprand_new_str(OP_FUNC, func->name), NULL, NULL);

        state_CompSt(rch2, func->data_func.ret, func->data_func.args);
    }
}

void state_ExtDecList(TreeNode* root, SymNode* type) {
    // VarDec

    SymNode* node = state_VarDec(rch0, type_dup_left(type));
    symtab_insert_now(node->name, node);

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
                bug_number++;
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

    struct_size_calc(type);

    const char* name = state_OptTag(rch1);
    if (name) {
        SymNode* entity = type_dup_right(type);
        symtab_insert(struct_table, name, entity);
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

        // root->data_int stores the array dimen
        // a[5][4][3] for [3, 4, 5]
        int* sizes = (int*)malloc(root->data_int * sizeof(int));

        // Find the size of each dimen
        TreeNode* elem = root;
        for (int i = 0; i < root->data_int; i++) {
            sizes[i] = elem->children[2]->data_int;
            elem = elem->children[0];
        }

        // elem is now VarDec -> ID
        SymNode* dimen = type_dup_left(type);
        dimen->name = elem->children[0]->data_str;

        for (int i = 0; i < root->data_int; i++) {
            SymNode* dimen_new = type_new_array(dimen);
            dimen_new->data_array.dimen = i + 1;
            dimen_new->data_array.size = sizes[i];
            dimen_new->name = dimen->name;
            dimen_new->size = dimen->size * sizes[i];
            dimen = dimen_new;
        }

        free(sizes);

        return dimen;
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

        struct_size_calc(args);

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
    symtab_insert_now(arg->name, arg);

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

            CODE_INSERT(CODE_PARAM, 0, OP_NEW_VAR(name), NULL, NULL);

            // Set param flag
            SymNode* arg = type_dup_left(args->data_struct.types[i]);
            arg->is_param = 1;

            symtab_insert_now(name, arg);
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
        state_Exp(rch0, 0);

    } else if (rsz == 3) {
        // RETURN Exp SEMI
        int target = tmpvar_new();
        ExpRet_t exp = state_Exp(rch1, target);
        CODE_INSERT(CODE_RET, 0, OP_NEW_TEMP(target), NULL, NULL);

    } else if (rch0->state_type == STATE_WHILE) {
        // WHILE LP Exp RP Stmt
        int label1 = label_new();
        int label2 = label_new();
        int label3 = label_new();

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label1), NULL, NULL);

        state_Cond(rch2, label2, label3);

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label2), NULL, NULL);

        state_Stmt(rch4, ret);

        CODE_INSERT(CODE_GOTO, 0, OP_NEW_LABEL(label1), NULL, NULL);

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label3), NULL, NULL);

    } else if (rsz == 5) {
        // IF    LP Exp RP Stmt
        int label1 = label_new();
        int label2 = label_new();

        state_Cond(rch2, label1, label2);

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label1), NULL, NULL);

        state_Stmt(rch4, ret);

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label2), NULL, NULL);

    } else {
        // IF    LP Exp RP Stmt ELSE Stmt
        int label1 = label_new();
        int label2 = label_new();
        int label3 = label_new();

        state_Cond(rch2, label1, label2);

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label1), NULL, NULL);

        state_Stmt(rch4, ret);

        CODE_INSERT(CODE_GOTO, 0, OP_NEW_LABEL(label3), NULL, NULL);

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label2), NULL, NULL);

        state_Stmt(rch6, ret);

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label3), NULL, NULL);
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

    // printf("%s: %d\n", node->name, node->size);

    // Insert

    symtab_insert_now(node->name, node);

    if (type_pos) {
        // Struct
        *type_pos = node;
    } else {
        // Normal Dec
        if (node->type == TYPE_STRUCT || node->type == TYPE_ARRAY)
            CODE_INSERT(CODE_DEC, node->size, OP_NEW_VAR(node->name), NULL, NULL);
    }

    // VarDec
    if (root->size == 3) {
        // VarDec ASSIGNOP Exp

        int target = tmpvar_new();
        state_Exp(rch2, target);

        CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_VAR(node->name), OP_NEW_TEMP(target), NULL);
    }
}

ExpRet_t state_Exp(TreeNode* root, int target) {
    // Must return in conditions

    if (rch0->state_type == STATE_INT) {
        // INT
        SymNode* type = type_new_int(rch0->data_int);
        type->is_right = 1;

        CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_TEMP(target), OP_NEW_CONST(rch0->data_int), NULL);

        return ExpRet_val(type);
    } else if (rch0->state_type == STATE_FLOAT) {
        // NO FLOAT CONST ALLOWED
        bug_number++;

        // FLOAT
        SymNode* type = type_new_float(rch0->data_float);
        type->is_right = 1;
        return ExpRet_val(type);
    }

    if (rch0->state_type == STATE_ID) {
        SymNode* id = symtab_lookup_all(rch0->data_str);
        if (rsz == 1) {
            // ID
            int addr_tmp = tmpvar_new();

            if (id->is_param && (id->type == TYPE_ARRAY || id->type == TYPE_STRUCT)) {
                CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_TEMP(addr_tmp), OP_NEW_VAR(rch0->data_str), NULL);
            } else {
                CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_TEMP(target), OP_NEW_VAR(rch0->data_str), NULL);

                CODE_INSERT(CODE_GETADDR, 0, OP_NEW_TEMP(addr_tmp), OP_NEW_VAR(rch0->data_str), NULL);
            }

            return ExpRet_addr(id, addr_tmp);
        }
        // ID LP Args RP
        // ID LP RP

        if (rsz == 3) {
            // ID LP RP

            if (!strcmp("read", rch0->data_str)) {
                CODE_INSERT(CODE_READ, 0, OP_NEW_TEMP(target), NULL, NULL);
            } else {
                CODE_INSERT(CODE_CALL, 0, OP_NEW_TEMP(target), IrOprand_new_str(OP_FUNC, rch0->data_str), NULL);
            }

            return ExpRet_val(type_dup_right(id->data_func.ret));
        } else {
            // ID LP Args RP

            SymNode* args = type_new_struct(rch2->data_int);

            int* tmp_pos = (int*)malloc(sizeof(int) * rch2->data_int);
            state_Args(rch2, args->data_struct.types, tmp_pos);

            if (!strcmp("write", rch0->data_str)) {
                CODE_INSERT(CODE_WRITE, 0, OP_NEW_TEMP(tmp_pos[0]), NULL, NULL);
                CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_TEMP(target), OP_NEW_CONST(0), NULL);
            } else {
                for (int i = args->data_struct.size - 1; i >= 0; i--) {
                    CODE_INSERT(CODE_ARG, 0, OP_NEW_TEMP(tmp_pos[i]), NULL, NULL);
                }

                free(tmp_pos);

                CODE_INSERT(CODE_CALL, 0, OP_NEW_TEMP(target), IrOprand_new_str(OP_FUNC, rch0->data_str), NULL);
            }

            return ExpRet_val(type_dup_right(id->data_func.ret));
        }
    }

    if (rch0->state_type == STATE_LP) {
        // LP Exp RP
        // No code, just pass
        return state_Exp(rch1, target);
    }

    if (rch0->state_type == STATE_NOT || rch1->state_type == STATE_RELOP || rch1->state_type == STATE_AND ||
        rch1->state_type == STATE_OR) {
        // NOT, RELOP, AND, OR

        int label1 = label_new();
        int label2 = label_new();

        CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_TEMP(target), OP_NEW_CONST(0), NULL);

        state_Cond(root, label1, label2);

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label1), NULL, NULL);

        CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_TEMP(target), OP_NEW_CONST(1), NULL);

        CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label2), NULL, NULL);

        return ExpRet_val(type_new_int(0));
    }

    int tmp_1 = tmpvar_new();
    int tmp_2 = tmpvar_new();

    // if (rsz == )
    ExpRet_t exp1;
    if (rch0->state_type == STATE_Exp) {
        // others
        exp1 = state_Exp(rch0, tmp_1);
    } else {
        // MINUS Exp
        // NOT Exp
        exp1 = state_Exp(rch1, tmp_1);
    }

    ExpRet_t exp2;
    if (rsz >= 3 && rch2->state_type == STATE_Exp) {
        exp2 = state_Exp(rch2, tmp_2);
    }

    if (rch0->state_type == STATE_MINUS || rch1->state_type == STATE_PLUS || rch1->state_type == STATE_MINUS ||
        rch1->state_type == STATE_STAR || rch1->state_type == STATE_DIV) {
        // NEG, PLUS, MINUS, STAR, DIV
        int code_type = CODE_ASSIGN;
        IrOprand *op_left = NULL, *op_right = NULL;
        if (rch0->state_type == STATE_MINUS) {
            code_type = CODE_SUB;
            op_left = OP_NEW_CONST(0);
            op_right = OP_NEW_TEMP(tmp_1);
        } else {
            op_left = OP_NEW_TEMP(tmp_1);
            op_right = OP_NEW_TEMP(tmp_2);
            if (rch1->state_type == STATE_PLUS)
                code_type = CODE_ADD;
            else if (rch1->state_type == STATE_MINUS)
                code_type = CODE_SUB;
            else if (rch1->state_type == STATE_STAR)
                code_type = CODE_MUL;
            else if (rch1->state_type == STATE_DIV)
                code_type = CODE_DIV;
        }
        CODE_INSERT(code_type, 0, OP_NEW_TEMP(target), op_left, op_right);
        return ExpRet_val(type_dup_left(exp1.node));
    }

    if (rch1->state_type == STATE_DOT) {
        // Exp DOT ID
        int offset = 0;

        SymNode* ret = NULL;
        for (int i = 0; i < exp1.node->data_struct.size; i++) {
            SymNode* ptr = exp1.node->data_struct.types[i];
            if (!strcmp(ptr->name, rch2->data_str)) {
                ret = ptr;
                break;
            } else {
                offset += ptr->size;
            }
        }

        int tmp_addr = tmpvar_new();
        CODE_INSERT(CODE_ADD, 0, OP_NEW_TEMP(tmp_addr), OP_NEW_TEMP(exp1.addr), OP_NEW_CONST(offset));

        CODE_INSERT(CODE_GETDATA, 0, OP_NEW_TEMP(target), OP_NEW_TEMP(tmp_addr), NULL);

        return ExpRet_addr(ret, tmp_addr);
    }

    if (rch1->state_type == STATE_LB) {
        // Exp LB Exp RB

        int tmp_addr = tmpvar_new();
        CODE_INSERT(CODE_MUL, 0, OP_NEW_TEMP(tmp_addr), OP_NEW_TEMP(tmp_2),
                    OP_NEW_CONST(exp1.node->data_array.next->size));

        CODE_INSERT(CODE_ADD, 0, OP_NEW_TEMP(tmp_addr), OP_NEW_TEMP(tmp_addr), OP_NEW_TEMP(exp1.addr));

        CODE_INSERT(CODE_GETDATA, 0, OP_NEW_TEMP(target), OP_NEW_TEMP(tmp_addr), NULL);

        return ExpRet_addr(exp1.node->data_array.next, tmp_addr);
    }

    if (rch1->state_type == STATE_ASSIGNOP) {
        // Exp ASSIGNOP Exp

        if (exp1.addr == -1) {
            // Must be variable
            CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_VAR(exp1.node->name), OP_NEW_TEMP(tmp_2), NULL);
        } else {
            // Addr
            CODE_INSERT(CODE_SETDATA, 0, OP_NEW_TEMP(exp1.addr), OP_NEW_TEMP(tmp_2), NULL);
        }

        CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_TEMP(target), OP_NEW_TEMP(tmp_2), NULL);

        return ExpRet_val(exp1.node);
    }

    CODE_INSERT(CODE_ASSIGN, 0, OP_NEW_TEMP(target), OP_NEW_CONST(-1), NULL);
    return ExpRet_addr(type_new_invalid(), -1);
}

void state_Cond(TreeNode* root, int label_true, int label_false) {
    // Must return in condition once match
    if (rsz == 2 && rch0->state_type == STATE_NOT) {
        // NOT Exp

        state_Cond(rch1, label_false, label_true);

        return;
    } else if (rsz == 3) {
        if (rch1->state_type == STATE_RELOP) {
            // Exp1 RELOP Exp2

            int tmp1 = tmpvar_new();
            int tmp2 = tmpvar_new();

            state_Exp(rch0, tmp1);
            state_Exp(rch2, tmp2);

            CODE_INSERT(CODE_GOCOND, rch1->data_int, OP_NEW_TEMP(tmp1), OP_NEW_TEMP(tmp2), OP_NEW_LABEL(label_true));

            CODE_INSERT(CODE_GOTO, 0, OP_NEW_LABEL(label_false), NULL, NULL);

            return;
        } else if (rch1->state_type == STATE_AND) {
            // Exp1 and Exp2

            int label1 = label_new();

            state_Cond(rch0, label1, label_false);
            CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label1), NULL, NULL);

            state_Cond(rch2, label_true, label_false);

            return;
        } else if (rch1->state_type == STATE_OR) {
            // Exp1 or Exp2

            int label1 = label_new();

            state_Cond(rch0, label_true, label1);
            CODE_INSERT(CODE_LABEL, 0, OP_NEW_LABEL(label1), NULL, NULL);

            state_Cond(rch2, label_true, label_false);

            return;
        }

        // Fall to others
    }

    // Others

    int tmp1 = tmpvar_new();
    state_Exp(root, tmp1);
    CODE_INSERT(CODE_GOCOND, RELOP_NE, OP_NEW_TEMP(tmp1), OP_NEW_CONST(0), OP_NEW_LABEL(label_true));

    CODE_INSERT(CODE_GOTO, 0, OP_NEW_LABEL(label_false), NULL, NULL);

    return;
}

void state_Args(TreeNode* root, SymNode** type_pos, int* tmp_pos) {
    // Exp
    *tmp_pos = tmpvar_new();
    ExpRet_t exp = state_Exp(rch0, *tmp_pos);
    *type_pos = exp.node;
    if (exp.node->type == TYPE_STRUCT || exp.node->type == TYPE_ARRAY) {
        *tmp_pos = exp.addr;
    }
    // *type_pos = state_Exp(rch0, *tmp_pos).node;
    if (rsz == 3) {
        // Exp COMMA Args
        state_Args(rch2, type_pos + 1, tmp_pos + 1);
    }
}
