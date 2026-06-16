#include "../include/vm.h"
#include <stdlib.h>
#include <stdio.h>

void vm_init(VM *vm){
    vm -> code_count = 0;
    vm -> stack_top = 0;
}

void vm_emit(VM *vm, OpCode op, double operand){
    vm -> code[vm->code_count].code = op;
    vm -> code[vm->code_count].operand = operand;
    vm -> code_count++;

}

void vm_run(VM *vm){
    for(int i = 0; i < vm -> code_count; i++){
        Instruction ins = vm -> code[i];

        switch(ins.code){
            case OP_PUSH:
                vm -> stack[vm -> stack_top++] = ins.operand;
                break;
            
            case OP_ADD: {
                double a = vm -> stack[--vm -> stack_top];
                double b = vm -> stack[--vm -> stack_top];
                vm -> stack[vm -> stack_top++] = a + b;
                break;
            }

            case OP_SUB: {
                double a = vm -> stack[--vm -> stack_top];
                double b = vm -> stack[--vm -> stack_top];
                vm -> stack[vm -> stack_top++] = b - a;
                break;
            }

            case OP_MUL: {
                double a = vm -> stack[--vm -> stack_top];
                double b = vm -> stack[--vm -> stack_top];
                vm -> stack[vm -> stack_top++] = a * b;
                break;
            }

            case OP_DIV: {
                double a = vm -> stack[--vm -> stack_top];
                double b = vm -> stack[--vm -> stack_top];
                if(a == 0){
                    perror("Error: cannot divide by 0");
                    exit(1);
                }

                vm -> stack[vm -> stack_top++] = b / a;
                break;
            }

            case OP_HALT:
                printf("%.2f\n", vm -> stack[vm -> stack_top - 1]);
                return;
                
        }
    }
    
}