#include <stdlib.h>
#include "../include/codegen.h"

void codegen(ASTNode *node, VM *vm){
    if(!node) return;
    if(node -> type == NODE_NUMBER){
        vm_emit(vm, OP_PUSH, node -> number);
        return;
    }


    // post order: left, right, operator
    codegen(node -> binop.left, vm);
    codegen(node -> binop.right, vm);

    switch(node -> binop.op){
        case '+': vm_emit(vm, OP_ADD, 0); break;
        case '-': vm_emit(vm, OP_SUB, 0); break;
        case '*': vm_emit(vm, OP_MUL, 0); break;
        case '/': vm_emit(vm, OP_DIV, 0); break;
    }
}

