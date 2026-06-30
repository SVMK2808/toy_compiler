#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ast.h"

ASTNode *make_number(double value){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_NUMBER;
    node -> number = value;
    return node;
}

ASTNode *make_binop(char op, ASTNode *left, ASTNode *right){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_BINOP;
    node -> binop.op = op;
    node -> binop.left = left;
    node -> binop.right = right;
    return node;
}


ASTNode *make_ident(const char* name){
    ASTNode *t = malloc(sizeof(ASTNode));
    t -> type = NODE_IDENT;
    strcpy(t -> ident, name);
    return t;
}

ASTNode *make_let(const char* name, ASTNode *value){
    ASTNode *t = malloc(sizeof(ASTNode));
    t -> type = NODE_LET;
    t -> let. value = value;
    strcpy(t -> let.name, name);
    return t;
}


ASTNode *make_print(ASTNode *expr){
    ASTNode *t = malloc(sizeof(ASTNode));
    t ->type = NODE_PRINT;
    t -> print = expr;
    return t;
}

ASTNode *make_if(ASTNode *condition,
                 ASTNode **then_body, int then_count,
                 ASTNode **else_body, int else_count){
                    ASTNode *node = malloc(sizeof(ASTNode));
                    node -> type = NODE_IF;
                    node -> if_else.condition = condition;
                    node -> if_else.then_body = then_body;
                    node -> if_else.then_count = then_count;
                    node -> if_else.else_body = else_body;
                    node -> if_else.else_count = else_count;
                    return node;
                 }
ASTNode *make_compare(char *op, ASTNode *left, ASTNode *right){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_COMPARE;
    strncpy(node -> compare.op, op, 3);
    node -> compare.left = left;
    node -> compare.right = right;
    return node;

}

ASTNode *make_assign(const char* name, ASTNode *value){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_ASSIGN;
    strncpy(node -> assign.name, name, 64);
    node -> assign.value = value;
    return node;
}

ASTNode *make_while(ASTNode *condition, ASTNode **body, int body_count){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_WHILE;
    node -> while_loop.condition = condition;
    node -> while_loop.body = body;
    node -> while_loop.body_count = body_count;
    return node;
}

ASTNode *make_do_while(ASTNode *condition, ASTNode **body, int body_count){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_DO_WHILE;
    node -> do_while.condition = condition;
    node -> do_while.body = body;
    node -> do_while.body_count = body_count;
    return node;
}

ASTNode *make_for(ASTNode *init, ASTNode *condition, ASTNode *incr, ASTNode **body, int body_count){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_FOR;
    node -> for_loop.init = init;
    node -> for_loop.condition = condition;
    node -> for_loop.increment = incr;
    node -> for_loop.body = body;
    node -> for_loop.body_count = body_count;
    return node;
}

ASTNode *make_array_lit(ASTNode **elements, int count){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_ARRAY_LIT;
    node -> array_lit.elements = elements;
    node -> array_lit.element_count = count;
    return node;
}

ASTNode *make_array_index(ASTNode *array, ASTNode *index){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_ARRAY_INDEX;
    node -> array_index.array = array;
    node -> array_index.index = index;
    return node;
}

ASTNode *make_array_assign(const char *name, ASTNode *index, ASTNode *value){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_ARRAY_ASSIGN;
    node -> array_assign.index = index;
    node -> array_assign.value = value;
    strncpy(node -> array_assign.name, name, 64);
    return node;
}

ASTNode *make_assert(ASTNode *condition){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_ASSERT;
    node -> assertion.condition = condition;
    return node;
}

ASTNode *make_func_def(const char *name, char params[][64], int param_count, ASTNode **body, int body_count){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_FUNC_DEF;
    strncpy(node -> func_def.name, name, 64);
    node -> func_def.param_count = param_count;
    for(int i = 0; i < param_count; i++){
        strncpy(node -> func_def.params[i], params[i], 64);
    }
    node -> func_def.body = body;
    node -> func_def.body_count = body_count;
    return node;
}

ASTNode *make_func_call(const char *name, ASTNode **args, int arg_count){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_FUNC_CALL;
    strncpy(node -> func_call.name, name, 64);
    node -> func_call.args = args;
    node -> func_call.arg_count = arg_count;
    return node;
}

ASTNode *make_return(ASTNode *value){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_RETURN;
    node -> ret_val = value;
    return node;

}

ASTNode *make_logical(const char *op, ASTNode *left, ASTNode *right){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_LOGICAL;
    node -> logical.left = left;
    node -> logical.right = right;
    strncpy(node -> logical.op, op, 3);
    return node;
}

ASTNode *make_unary(char op, ASTNode *operand){
    ASTNode *node = malloc(sizeof(ASTNode));
    node -> type = NODE_UNARY;
    node -> unary.op = op;
    node -> unary.operand = operand;
    return node;
}

void print_ast(ASTNode *node, int depth){
    if(node == NULL) return;

    for(int i = 0; i < depth; i++) printf("  ");

    switch(node -> type){
        case NODE_PRINT: 
            printf("PRINT\n");
            print_ast(node -> print, depth + 1);
            break;

        case NODE_LET:
            printf("LET(%s)\n", node -> let.name);
            print_ast(node -> let.value, depth + 1);
            break;

        case NODE_IDENT:
            printf("IDENT(%s)\n", node -> ident);
            break;
        
        case NODE_NUMBER:
            printf("NUMBER(%.0f)\n", node -> number);
            break;
        
        case NODE_BINOP:
            printf("BINOP(%c)\n", node -> binop.op);
            print_ast(node -> binop.left, depth + 1);
            print_ast(node -> binop.right, depth + 1);
            break;

        case NODE_LOGICAL:
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("LOGICAL(%s): \n", node -> logical.op);
            print_ast(node -> logical.left, depth + 1);
            print_ast(node -> logical.right, depth + 1);
            break;
        
        case NODE_UNARY:
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("UNARY(%c): \n", node -> unary.op);
            print_ast(node -> unary.operand, depth + 1);
            break;

        case NODE_COMPARE:
            printf("COMPARE(%s):\n", node -> compare.op);
            print_ast(node -> compare.left, depth + 1);
            print_ast(node -> compare.right, depth + 1);
            break;
        
        case NODE_IF:
            printf("IF\n");
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("CONDITION:\n");
            print_ast(node -> if_else.condition, depth + 2);

            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("THEN: \n");
            for(int i = 0; i < node -> if_else.then_count; i++)
                print_ast(node -> if_else.then_body[i], depth + 2);

            if(node -> if_else.else_body){
                for(int i = 0; i < depth + 1; i++) printf(" ");
                printf("ELSE: \n");
                for(int i = 0; i < node -> if_else.else_count; i++)
                    print_ast(node -> if_else.else_body[i], depth + 2);
            }
            break;

        case NODE_ASSIGN:
            printf("ASSIGN(%s)\n", node -> assign.name);
            print_ast(node -> assign.value, depth + 1);
            break;
        
        case NODE_FOR:
            printf("FOR: \n");
            for(int i = 0; i< depth + 1; i++) printf(" ");
            printf("INIT: \n");
            print_ast(node -> for_loop.init, depth + 2);
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("CONDITION: \n");
            print_ast(node -> for_loop.condition, depth + 2);
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("INCREMENT: \n");
            print_ast(node -> for_loop.increment, depth + 2);
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("BODY: \n");
            for(int i = 0; i < node -> for_loop.body_count; i++)
                print_ast(node -> for_loop.body[i], depth + 2);
            break;

        case NODE_WHILE:
            printf("WHILE\n");
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("CONDITION: \n");
            print_ast(node -> while_loop.condition, depth + 2);
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("BODY: \n");
            for(int i = 0; i < node -> while_loop.body_count; i++){
                print_ast(node -> while_loop.body[i], depth + 2);
            }
        break;

        case NODE_DO_WHILE:
            printf("DO_WHILE\n");
            for(int i = 0; i < depth + 1; i++)printf(" ");
            printf("BODY: \n");
            for(int i = 0; i < node -> do_while.body_count; i++)
                print_ast(node -> do_while.body[i], depth + 2);

            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("CONDITION: \n");
            print_ast(node -> do_while.condition, depth + 2);
            break;
        
        case NODE_ARRAY_LIT:
            printf("ARRAY_LIT elements = %d\n", node -> array_lit.element_count);
            for(int i = 0; i < node -> array_lit.element_count; i++){
                print_ast(node -> array_lit.elements[i], depth + 1);
            }
            break;
        
        case NODE_ARRAY_INDEX:
            printf("ARRAY_INDEX\n");
            print_ast(node -> array_index.index, depth + 1);
            break;
        
        case NODE_ARRAY_ASSIGN:
            printf("ARRAY_DESIGN(%s)\n", node -> array_assign.name);
            print_ast(node -> array_assign.index, depth + 1);
            print_ast(node -> array_assign.value, depth + 1);
            break;

        case NODE_ASSERT: 
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("ASSERT\n");
            print_ast(node -> assertion.condition, depth + 1);
            break;

        case NODE_FUNC_DEF:
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("FUNC_DEF(%s) params = [", node -> func_def.name);
            for(int i = 0; i < node -> func_def.param_count; i++)
                printf("%s%s", node -> func_def.params[i], i < node -> func_def.param_count - 1 ? ", ": "");
            printf("]\n");
            for(int i = 0; i < node -> func_def.body_count; i++)
                print_ast(node -> func_def.body[i], depth + 1);
            break;
        
        case NODE_FUNC_CALL:
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("FUNC_CALL(%s)\n", node -> func_call.name);
            for(int i = 0; i < node -> func_call.arg_count; i++)
                print_ast(node -> func_call.args[i], depth + 2);
            break;

        case NODE_RETURN:
            for(int i = 0; i < depth + 1; i++) printf(" ");
            printf("RETURN\n");
            print_ast(node -> ret_val, depth + 1);
            break;
    }
    
    
}

void free_ast(ASTNode *node){
    if(!node) return;

    switch(node -> type){
        case NODE_LET:
            free_ast(node -> let.value);
            break;
        
        case NODE_PRINT:
            free_ast(node -> print);
            break;

        case NODE_BINOP:
            free_ast(node -> binop.left);
            free_ast(node -> binop.right);
            break;

        case NODE_LOGICAL:
            free_ast(node -> logical.left);
            free_ast(node -> logical.right);
            break;
        
        case NODE_UNARY:
            free_ast(node -> unary.operand);
            break;

        case NODE_COMPARE:
            free_ast(node -> compare.left);
            free_ast(node -> compare.right);
            break;
        
        case NODE_IF:
            free_ast(node -> if_else.condition);
            for(int i = 0; i< node -> if_else.then_count; i++)
                free_ast(node -> if_else.then_body[i]);
            free(node -> if_else.then_body);

            for(int i = 0; i < node -> if_else.else_count; i++)
                free_ast(node -> if_else.else_body[i]);
            free(node -> if_else.else_body);
            break;
        
        case NODE_ASSIGN:
            free_ast(node -> assign.value);
            break;

        case NODE_FOR:
            free_ast(node -> for_loop.init);
            free_ast(node -> for_loop.condition);
            free_ast(node -> for_loop.increment);
            for(int i = 0; i < node -> for_loop.body_count; i++)
                free_ast(node -> for_loop.body[i]);
            free(node -> for_loop.body);
            break;

        case NODE_WHILE:
            free_ast(node -> while_loop.condition);
            for(int i = 0; i< node -> while_loop.body_count; i++)
                free_ast(node -> while_loop.body[i]);
            free(node -> while_loop.body);
            break;

        case NODE_DO_WHILE:
            for(int i = 0; i < node -> do_while.body_count; i++)
                free_ast(node -> do_while.body[i]);
            free(node -> do_while.body);
            free_ast(node -> do_while.condition);
            break;
        
        case NODE_ARRAY_LIT:
            for(int i = 0; i < node -> array_lit.element_count; i++)
                free_ast(node -> array_lit.elements[i]);
            free(node -> array_lit.elements);
            break;
        
        case NODE_ARRAY_INDEX:
            free_ast(node -> array_index.array);
            free_ast(node -> array_index.index);
            break;

        case NODE_ARRAY_ASSIGN:
            free_ast(node -> array_assign.index);
            free_ast(node -> array_assign.value);
            break;
        
        case NODE_ASSERT:
            free_ast(node -> assertion.condition);
            break;

        case NODE_FUNC_DEF:
            for(int i = 0; i < node -> func_def.body_count; i++)
                free_ast(node -> func_def.body[i]);
            free(node -> func_def.body);
            break;
        
        case NODE_FUNC_CALL:
            for(int i = 0; i< node -> func_call.arg_count; i++)
                free_ast(node -> func_call.args[i]);
            free(node -> func_call.args);
            break;
        
        case NODE_RETURN:
            free_ast(node -> ret_val);
            break;

        /*These two are just to silence the compiler warning. */
        case NODE_IDENT:    
        case NODE_NUMBER:
        break;
    }
    free(node);  // covers for NODE_NUMBER and NODE_IDENT as they are individual nodes
}