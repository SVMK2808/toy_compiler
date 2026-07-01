#include <stdlib.h>
#include <string.h>
#include "../include/codegen.h"

void codegen(ASTNode *node, VM *vm, bool in_func){
    if(!node) return;
    switch (node -> type) {
        case NODE_NUMBER:
            vm_emit(vm, OP_PUSH , node -> number, NULL);
            break;
        
        case NODE_IDENT:
            vm_emit(vm, in_func ? OP_LOAD_LOCAL : OP_LOAD, 0, node -> ident);
            break;
        
        case NODE_LET:
            codegen(node -> let.value, vm, in_func);
            vm_emit(vm, in_func ? OP_STORE_LOCAL: OP_STORE, 0, node -> let.name);
            break;
        
        case NODE_PRINT:
            codegen(node -> print, vm, in_func);
            vm_emit(vm, OP_PRINT, 0, NULL);
            break;
        
        case NODE_BINOP:
            codegen(node -> binop.left, vm, in_func);
            codegen(node -> binop.right, vm, in_func);
            switch(node -> binop.op){
                case '-' : vm_emit(vm, OP_SUB, 0, NULL); break;
                case '+' : vm_emit(vm, OP_ADD, 0, NULL); break;
                case '*' : vm_emit(vm, OP_MUL, 0, NULL); break;
                case '/' : vm_emit(vm, OP_DIV, 0, NULL); break;
            }
            break;

        case NODE_UNARY:
            if(node -> unary.op == '-'){
                // Unary arithmetic negation: compile expr as (0.0 - expr)
                vm_emit(vm, OP_PUSH, 0.0, NULL);
                codegen(node -> unary.operand, vm, in_func);
                vm_emit(vm, OP_SUB, 0, NULL);
            }else if(node -> unary.op == '!'){
                // Unary logical negation
                codegen(node -> unary.operand, vm, in_func);
                vm_emit(vm, OP_NOT, 0, NULL);
            }
            break;
            
        case NODE_LOGICAL: {
            if(strcmp(node -> logical.op, "&&") == 0){
                //1. Evaluate left operand
                codegen(node -> logical.left, vm, in_func);
                //2. Jump to false_label if left is false
                int jmp_false1 = vm -> code_count;
                vm_emit(vm, OP_JMP_IF_FALSE, 0, NULL);
                //3. Evaluate right operand
                codegen(node -> logical.right, vm, in_func);

                //4. Jump to false_label if right is false
                int jmp_false2 = vm -> code_count;
                vm_emit(vm, OP_JMP_IF_FALSE, 0, NULL);

                //5. If both true, push 1.0 and jump to end
                vm_emit(vm, OP_PUSH, 1.0, NULL);
                int jmp_end = vm -> code_count;
                vm_emit(vm, OP_JMP, 0, NULL);

                //6. Define false label here 
                vm -> code[jmp_false1].operand = vm -> code_count;
                vm -> code[jmp_false2].operand = vm -> code_count;
                vm_emit(vm, OP_PUSH, 0.0, NULL);

                //7. Define end_label here
                vm -> code[jmp_end].operand = vm -> code_count;
            }else if(strcmp(node -> logical.op, "||") == 0){
                // 1. Evaluate left operand
                codegen(node -> logical.left, vm, in_func);
                //2. Jump to eval_b_label if left is false
                int jmp_eval_b = vm -> code_count;
                vm_emit(vm, OP_JMP_IF_FALSE, 0, NULL);

                //3. If left is true, push 1.0 and jump to end
                vm_emit(vm, OP_PUSH, 1.0, NULL);
                int jmp_end1 = vm -> code_count;
                vm_emit(vm, OP_JMP, 0, NULL);

                //4. eval_b_label: evaluate right operand
                vm -> code[jmp_eval_b].operand = vm -> code_count;
                codegen(node -> logical.right, vm, in_func);

                //5. Jump to false_label if right is false
                int jmp_false = vm -> code_count;
                vm_emit(vm, OP_JMP_IF_FALSE, 0, NULL);

                //6. If right is true, push 1.0 and jump to end
                vm_emit(vm, OP_PUSH, 1.0, NULL);
                int jmp_end2 = vm -> code_count;
                vm_emit(vm, OP_JMP, 0, NULL);

                //7. false label: push 0.0
                vm -> code[jmp_false].operand = vm -> code_count;
                vm_emit(vm, OP_PUSH, 0.0, NULL);

                //8. end label (bpth successful ends jump here, after the 0.0 push)
                vm -> code[jmp_end1].operand = vm -> code_count;
                vm -> code[jmp_end2].operand = vm -> code_count;
            }
            break;
        }

            case NODE_COMPARE:
                codegen(node -> compare.left, vm, in_func);
                codegen(node -> compare.right, vm, in_func);
                if     (strcmp(node -> compare.op, ">") == 0) vm_emit(vm, OP_GT, 0, NULL);
                else if(strcmp(node -> compare.op, "<") == 0) vm_emit(vm, OP_LT, 0, NULL);
                else if(strcmp(node -> compare.op, "==") == 0) vm_emit(vm, OP_EQ, 0, NULL);
                else if(strcmp(node -> compare.op, "<=") == 0) vm_emit(vm, OP_LE, 0, NULL);
                else if(strcmp(node -> compare.op, ">=") == 0) vm_emit(vm, OP_GE, 0, NULL);
                break;

            case NODE_IF:{
                // 1. emit condition
                codegen(node -> if_else.condition, vm, in_func);

                // 2. emit JMP_IF_FALSE - we don't know the target yet, patch later
                int jmp_if_false_idx = vm -> code_count;
                vm_emit(vm, OP_JMP_IF_FALSE, 0, NULL);

                // 3. emit then body
                for(int i = 0; i < node -> if_else.then_count; i++){
                    codegen(node -> if_else.then_body[i], vm, in_func);
                }

                // 4. emit JMP to skip else - patch later
                int jmp_idx =  vm -> code_count;
                vm_emit(vm, OP_JMP, 0, NULL);

                // 5. patch JMP_IF_FALSE to point here (start of else)
                vm -> code[jmp_if_false_idx].operand = vm -> code_count;

                //6. emit else body
                if(node -> if_else.else_body){
                    for(int i = 0; i < node -> if_else.else_count; i++){
                        codegen(node -> if_else.else_body[i], vm, in_func);
                    }
                }

                //7. patch JMP to point here (after else)
                vm -> code[jmp_idx].operand = vm -> code_count;
                break;
            }

            case NODE_ASSIGN:
                codegen(node -> assign.value, vm, in_func);
                vm_emit(vm, OP_ASSIGN_VAR, 0, node -> assign.name);
                break;

            case NODE_FOR: {
                //1. emit init once (let i = 0 or i = 0)
                codegen(node -> for_loop.init, vm, in_func);

                //2. save top of loop
                int loop_start = vm -> code_count;

                //3. emit condition
                codegen(node -> for_loop.condition, vm, in_func);

                //4. JMP_IF_FALSE - patch later
                int jmp_if_false_idx = vm -> code_count;
                vm_emit(vm, OP_JMP_IF_FALSE, 0, NULL);

                //5. emit body
                for(int i = 0; i < node -> for_loop.body_count; i++)
                    codegen(node -> for_loop.body[i], vm, in_func);

                //6. emit increment
                codegen(node -> for_loop.increment, vm, in_func);

                //7. jump back to loop_start
                vm_emit(vm, OP_JMP, loop_start, NULL);

                //8. patch JMP_IF_FALSE to after the loop
                vm -> code[jmp_if_false_idx].operand = vm -> code_count;
                break;
            }
            case NODE_WHILE: {
                //1. Emit check BEFORE entering loop (initial invariant check)
                if(node -> while_loop.invariant){
                    codegen(node -> while_loop.invariant, vm, in_func);
                    vm_emit(vm, OP_ASSERT, 0, NULL);
                }
                //2. save the index we'll jump back to (top of the loop)
                int loop_start = vm -> code_count;

                // 3. emit condition code
                codegen(node -> while_loop.condition, vm, in_func);

                // 4.. emit JMP_IF_FALSE - don't know target yet, patch later.
                int jmp_if_false_idx = vm -> code_count;
                vm_emit(vm, OP_JMP_IF_FALSE, 0, NULL);

                // 5. emit loop body
                for(int i = 0; i < node -> while_loop.body_count; i++){
                    codegen(node -> while_loop.body[i], vm, in_func);
                }

                //6. Emit check AFTER loop body (iteration invariant check)
                if(node -> while_loop.invariant){
                    codegen(node -> while_loop.invariant, vm, in_func);
                    vm_emit(vm, OP_ASSERT, 0, NULL);
                }

                //7. jump back to loop_start (known now, no patching needed)
                vm_emit(vm, OP_JMP, loop_start, NULL);

                //8. patch JMP_IF_FALSE to point here (after the loop)
                vm -> code[jmp_if_false_idx].operand = vm -> code_count;
                break;
            }

            case NODE_DO_WHILE:{
                //1. save loop start (top of body)
                int loop_start = vm -> code_count;

                //2. emit body first
                for(int i = 0; i < node -> do_while.body_count; i++)
                    codegen(node -> do_while.body[i], vm, in_func);

                //3. emit condition
                codegen(node -> do_while.condition, vm, in_func);

                //4. If condition true, jump back to loop_start
                vm_emit(vm, OP_JMP_IF_FALSE, vm -> code_count + 2, NULL); // skip the JMP
                vm_emit(vm, OP_JMP, loop_start, NULL);
                break;
            }

            case NODE_FUNC_DEF: {
                // Jump over the function body during sequential execution
                int jmp_idx = vm -> code_count;
                vm_emit(vm, OP_JMP, 0, NULL);
                int start_addr = vm -> code_count;
                vm_register_func(vm, node -> func_def.name, start_addr, node -> func_def.param_count, node -> func_def.params);

                // Generate bytecode for function body (in_func = true)
                for(int i = 0; i < node -> func_def.body_count; i++){
                    codegen(node -> func_def.body[i], vm, true);
                }

                // Fallback return if function body doesn't end with explicit return.
                vm_emit(vm, OP_PUSH, 0.0, NULL);
                vm_emit(vm, OP_RETURN, 0, NULL);

                // Patch the initial jump to point here (after the function body).
                vm -> code[jmp_idx].operand = vm -> code_count;
                break;
            }

            case NODE_FUNC_CALL:
                // Evaluate arguments from left to right
                for(int i = 0; i < node -> func_call.arg_count; i++){
                    codegen(node -> func_call.args[i], vm, in_func);
                }

                // Emit call instruction with the function name
                vm_emit(vm, OP_CALL, 0, node -> func_call.name);
                break;
            
            case NODE_RETURN:
                codegen(node -> ret_val, vm, in_func);
                vm_emit(vm, OP_RETURN, 0, NULL);
                break;

            case NODE_ARRAY_LIT: {
                // Compile elements from left to right 
                for(int i = 0; i< node -> array_lit.element_count; i++){
                    codegen(node -> array_lit.elements[i], vm, in_func);
                }

                // Emit OP_NEW_ARRAY with count as double operand
                vm_emit(vm, OP_NEW_ARRAY, node -> array_lit.element_count, NULL);
                break;
            }

            case NODE_ARRAY_INDEX: {
                // 1. Compile array pointer lookup (pushes array's heap start index)
                codegen(node -> array_index.array, vm, in_func);

                // 2. Compile index expression (pushes index value)
                codegen(node -> array_index.index, vm, in_func);
                
                // 3. Emit OP_LOAD_ARRAY to pop pointer + index and load value from heap
                vm_emit(vm, OP_LOAD_ARRAY, 0, NULL);
                break;
            }

            case NODE_ARRAY_ASSIGN: {
                //1. Compile right - hand side value expression (pushes value)
                codegen(node -> array_assign.value, vm, in_func);

                //2. Emit load to put array pointer on stack 
                vm_emit(vm, in_func ? OP_LOAD_LOCAL : OP_LOAD, 0, node -> array_assign.name);

                // 3. Compile index expression (pushes index)
                codegen(node -> array_assign.index, vm, in_func);

                //4. Emit OP_STORE_ARRAY to store value in the array's heap slot
                vm_emit(vm, OP_STORE_ARRAY, 0, NULL);
                break;
            }

            case NODE_ASSERT: {
                // Compile assertion condition expression
                codegen(node -> assertion.condition, vm, in_func);
                
                // Emit OP_ASSERT
                vm_emit(vm, OP_ASSERT, 0.0, NULL);
                break; 
            }
    }
    

}