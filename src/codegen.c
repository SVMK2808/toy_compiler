#include <stdlib.h>
#include <string.h>
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

            case NODE_COMPARE:
                codegen(node -> compare.left, vm);
                codegen(node -> compare.right, vm);
                if     (strcmp(node -> compare.op, ">") == 0) vm_emit(vm, OP_GT, 0, NULL);
                else if(strcmp(node -> compare.op, "<") == 0) vm_emit(vm, OP_LT, 0, NULL);
                else if(strcmp(node -> compare.op, "==") == 0) vm_emit(vm, OP_EQ, 0, NULL);
                break;

            case NODE_IF:{
                // 1. emit condition
                codegen(node -> if_else.condition, vm);

                // 2. emit JMP_IF_FALSE - we don't know the target yet, patch later
                int jmp_if_false_idx = vm -> code_count;
                vm_emit(vm, OP_JMP_IF_FALSE, 0, NULL);

                // 3. emit then body
                for(int i = 0; i < node -> if_else.then_count; i++){
                    codegen(node -> if_else.then_body[i], vm);
                }

                // 4. emit JMP to skip else - patch later
                int jmp_idx =  vm -> code_count;
                vm_emit(vm, OP_JMP, 0, NULL);

                // 5. patch JMP_IF_FALSE to point here (start of else)
                vm -> code[jmp_if_false_idx].operand = vm -> code_count;

                //6. emit else body
                if(node -> if_else.else_body){
                    for(int i = 0; i < node -> if_else.else_count; i++){
                        codegen(node -> if_else.else_body[i], vm);
                    }
                }

                //7. patch JMP to point here (after else)
                vm -> code[jmp_idx].operand = vm -> code_count;
                break;
            }

            case NODE_WHILE: {
                //1. save the index we'll jump back to (top of the loop)
                int loop_start = vm -> code_count;

                // 2. emit condition code
                codegen(node -> while_loop.condition, vm);

                // 3. emit JMP_IF_FALSE - don't know target yet, patch later.
                int jmp_if_false_idx = vm -> code_count;
                vm_emit(vm, OP_JMP_IF_FALSE, 0, NULL);

                // 4. emit loop
                for(int i = 0; i < node -> while_loop.body_count; i++){
                    codegen(node -> while_loop.body[i], vm);
                }

                // 5. jump back to loop_start (known now, no patching needed)
                vm_emit(vm, OP_JMP, loop_start, NULL);

                // 6. patch JMP_IF_FALSE to point here (after the loop)
                vm -> code[jmp_if_false_idx].operand = vm -> code_count;
                break;
            }
    }
    

}