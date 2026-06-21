#include <stdlib.h>
#include "../include/codegen.h"

void codegen(ASTNode *node, VM *vm){
    if(!node) return;
    switch (node -> type) {
        case NODE_NUMBER:
            vm_emit(vm, OP_PUSH , node -> number, NULL);
            break;
        
        case NODE_IDENT:
            vm_emit(vm, OP_LOAD, 0, node -> ident);
            break;
        
        case NODE_LET:
            codegen(node -> let.value, vm);
            vm_emit(vm, OP_STORE, 0, node -> let.name);
            break;
        
        case NODE_PRINT:
            codegen(node -> print, vm);
            vm_emit(vm, OP_PRINT, 0, NULL);
            break;
        
        case NODE_BINOP:
            codegen(node -> binop.left, vm);
            codegen(node -> binop.right, vm);
            switch(node -> binop.op){
                case '-' : vm_emit(vm, OP_SUB, 0, NULL); break;
                case '+' : vm_emit(vm, OP_ADD, 0, NULL); break;
                case '*' : vm_emit(vm, OP_MUL, 0, NULL); break;
                case '/' : vm_emit(vm, OP_DIV, 0, NULL); break;
            }
            break;
    }
    

}