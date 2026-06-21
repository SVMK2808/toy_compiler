#include "../include/vm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void vm_init(VM *vm){
    vm -> code_count = 0;
    vm -> stack_top = 0;
    symtable_init(&vm -> symtable);
}

void vm_emit(VM *vm, OpCode op, double operand, const char* name){
    vm -> code[vm->code_count].code =       op;
    vm -> code[vm->code_count].operand =    operand;
    if(name){
        strncpy(vm -> code[vm -> code_count].name, name, 64);
    }else{
        vm -> code[vm -> code_count].name[0] = '\0';
    }
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
                double b = vm -> stack[--vm -> stack_top];
                double a = vm -> stack[--vm -> stack_top];
                if(b == 0){
                    fprintf(stderr, "Error: division by zero\n");
                    exit(1);
                }

                vm -> stack[vm -> stack_top++] = a / b;
                break;
            }

            case OP_STORE: {
                double val = vm -> stack[--vm -> stack_top];
                symtable_set(&vm -> symtable, ins.name, val);
                break;
            }

            case OP_LOAD: {
                if(!symtable_exists(&vm -> symtable, ins.name)){
                    printf("Error: undefined variable: %s", ins.name);
                    exit(1);
                }

                double val = symtable_get(&vm -> symtable, ins.name);
                vm -> stack[vm -> stack_top++] = val;
                break;
            }

            case OP_PRINT: {
                double val = vm -> stack[--vm -> stack_top];
                printf("%.02f\n", val);
                break;
            }
                
            case OP_HALT:
                return;
                
        }
    }
    
}