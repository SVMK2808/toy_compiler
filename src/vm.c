#include "../include/vm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void vm_init(VM *vm){
    vm -> code_count = 0;
    vm -> stack_top = 0;
    vm -> frame_top = -1;
    vm -> start_ip = 0;
    vm -> func_table.count = 0;
    vm -> heap_top = 0;
    vm -> func_table.count = 0;
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

void vm_register_func(VM *vm, const char *name, int code_start, int param_count, char params[][64]){
    FuncDef *f = &vm -> func_table.funcs[vm -> func_table.count++];
    strncpy(f -> name, name, 64);
    f -> code_start = code_start;
    f -> param_count = param_count;
    for(int i = 0; i < param_count; i++)
        strncpy(f -> params[i], params[i], 64);
}

void vm_run(VM *vm){
    for(int i = vm -> start_ip; i < vm -> code_count; i++){
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

            case OP_GT: {
                double  b = vm -> stack[--vm -> stack_top];
                double  a = vm -> stack[--vm -> stack_top];
                vm -> stack[vm -> stack_top++] = (a > b) ? 1.0 : 0.0;
                break;
            }

            case OP_LT: {
                double b = vm -> stack[--vm -> stack_top];
                double a = vm -> stack[--vm -> stack_top];
                vm -> stack[vm -> stack_top++] = (a < b) ? 1.0 : 0.0;
                break;
            }

            case OP_EQ: {
                double a = vm -> stack[--vm -> stack_top];
                double b = vm -> stack[--vm -> stack_top];
                vm -> stack[vm -> stack_top++] = (a == b)? 1.0 : 0.0;
                break;
            }

            case OP_JMP: {
                i = (int)ins.operand - 1; // -1 because loop will i++
                break;
            }

            case OP_JMP_IF_FALSE: {
                double val = vm -> stack[--vm -> stack_top];
                if(val == 0.0){
                    i = (int)ins.operand - 1;
                }
                break;
            }

            case OP_CALL: {
                // find the function by name
                FuncDef *fn = NULL;
                for(int j = 0; j < vm -> func_table.count; j++){
                    if(strcmp(vm -> func_table.funcs[j].name, ins.name) == 0){
                        fn = &vm -> func_table.funcs[j];
                        break;
                    }
                }

                if(!fn){
                    fprintf(stderr, "Error: undefined function '%s' \n", ins.name);
                    exit(1);
                }

                // push a new call frame
                vm -> frame_top++;
                CallFrame *frame = &vm -> frames[vm -> frame_top];
                frame -> return_addr = i + 1;
                symtable_init(&frame -> locals);

                // pop args right - to - left so param[0] gets the leftmost arg
                for(int j = fn -> param_count - 1; j >= 0; j--){
                    double val = vm -> stack[--vm -> stack_top];
                    symtable_set(&frame -> locals, fn -> params[j], val);
                }

                i = fn -> code_start - 1; // -1 because loop will i++
                break;
            }

            case OP_RETURN: {
                double ret = vm -> stack[--vm -> stack_top]; // grab return value
                int ret_addr = vm -> frames[vm -> frame_top].return_addr;
                vm -> frame_top--; // pop the call frame
                vm -> stack[vm -> stack_top++] = ret; //put the return value back
                i = ret_addr - 1; // -1 because loop will i++
                break;
            }

            case OP_LOAD_LOCAL: {
                CallFrame *frame = &vm -> frames[vm -> frame_top];
                if(symtable_exists(&frame -> locals, ins.name)){
                    vm -> stack[vm -> stack_top++] = symtable_get(&frame -> locals, ins.name);
                }else if(symtable_exists(&vm -> symtable, ins.name)){
                    vm -> stack[vm -> stack_top++] = symtable_get(&vm -> symtable, ins.name);
                }else {
                    fprintf(stderr, "Error: undefined variable '%s' \n", ins.name);
                    exit(1);
                }
                break;
            }

            case OP_STORE_LOCAL: {
                double val = vm -> stack[--vm -> stack_top];
                symtable_set(&vm -> frames[vm -> frame_top].locals, ins.name, val);
                break;
            }


            case OP_ASSIGN_VAR: {
                double val = vm -> stack[--vm -> stack_top];
                if(vm -> frame_top >= 0 && symtable_exists(&vm -> frames[vm -> frame_top].locals, ins.name)){
                    //variable is in the local scope of the function
                    symtable_set(&vm -> frames[vm -> frame_top].locals, ins.name, val);
                }else if(symtable_exists(&vm -> symtable, ins.name)){
                    // variable has a global scope
                    symtable_set(&vm -> symtable, ins.name, val);
                }else {
                    fprintf(stderr, "Error: undefined variable for assignment '%s' \n", ins.name);
                    exit(1);
                }
                break;
            }
                
            case OP_LE: {
                double b = vm -> stack[--vm -> stack_top];
                double a = vm -> stack[--vm -> stack_top];
                vm -> stack[vm -> stack_top++] = (a <= b) ? 1.0 : 0.0;
                break;
            }

            case OP_GE: {
                double b = vm -> stack[--vm -> stack_top];
                double a = vm -> stack[--vm -> stack_top];
                vm -> stack[vm -> stack_top++] = (a >= b) ? 1.0 : 0.0;
                break;
            }

            case OP_NOT: {
                double val = vm -> stack[--vm -> stack_top];
                vm -> stack[vm -> stack_top++] = (val == 0.0) ? 1.0 : 0.0;
                break;
            }

            case OP_NEW_ARRAY: {
                int count = (int)ins.operand;
                int start = vm -> heap_top;
                if(start + count > MAX_HEAP){
                    fprintf(stderr, "Error: VM heap overflow \n");
                    exit(1);
                }

                // Pop stack elements in reverse order since the rightmost element was pushed last
                for(int j = count - 1; j >= 0; j--){
                    vm -> heap[start + j] = vm -> stack[--vm -> stack_top];
                }

                vm -> heap_top += count;
                // Push the heap address (index) onto the stack 
                vm -> stack[vm -> stack_top++] = (double)start;
                break;
            }

            case OP_LOAD_ARRAY: {
                double idx = vm -> stack[--vm -> stack_top];
                double arr_ptr = vm -> stack[--vm -> stack_top];
                int final_ptr = (int)arr_ptr + (int)idx;
                if(final_ptr < 0 || final_ptr >= vm -> heap_top){
                    fprintf(stderr, "Error: Array index out of bounds: %d\n", final_ptr);
                    exit(1);
                }

                vm -> stack[vm -> stack_top++] = vm -> heap[final_ptr];
                break;
            }

            case OP_STORE_ARRAY: {
                double idx = vm -> stack[--vm -> stack_top];
                double arr_ptr = vm -> stack[--vm -> stack_top];
                double val = vm -> stack[--vm -> stack_top];
                int final_ptr = (int)arr_ptr + (int)idx;
                if(final_ptr < 0 || final_ptr >= vm -> heap_top){
                    fprintf(stderr, "Error: Array index out of bounds: %d\n", final_ptr);
                    exit(1);
                }

                vm -> heap[final_ptr] = val;
                break;
            }
            case OP_HALT:
                return;
                
        }
    }
    
}