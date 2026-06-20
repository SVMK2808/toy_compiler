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
    }
    free(node);  // covers for NODE_NUMBER and NODE_IDENT as they are individual nodes
}