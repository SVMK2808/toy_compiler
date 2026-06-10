#include <stdio.h>
#include <stdlib.h>
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

void print_ast(ASTNode *node, int depth){
    if(node == NULL) return;

    for(int i = 0; i < depth; i++) printf("  ");
    if(node -> type == NODE_NUMBER){
        printf("NUMBER(%.0f)\n", node-> number);
    }else{
        printf("BINOP(%c)\n", node -> binop.op);
        print_ast(node->binop.left, depth + 1);
        print_ast(node ->binop.right, depth + 1);
    }
}

void free_ast(ASTNode *node){
    if(!node) return;
    if(node -> type == NODE_BINOP){
        free_ast(node -> binop.left);
        free_ast(node -> binop.right);
    }
    free(node);
}