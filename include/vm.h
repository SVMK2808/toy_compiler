#ifndef VM_H
#define VM_H

#define MAX_STACK 256
#define MAX_CODE  1024

typedef enum {
    OP_PUSH,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_HALT
} OpCode;

typedef struct{
    OpCode code;
    double operand; //Only used for OP_PUSH
} Instruction;

typedef struct{
    Instruction code[MAX_CODE];   //bytecode instructions
    int         code_count;       //how many instructions

    double      stack[MAX_STACK]; //the stack
    int         stack_top;        //current stack pointer
    
} VM;

void vm_init(VM *vm);
void vm_emit(VM *vm, OpCode op, double operand);
void vm_run(VM *vm);

#endif //VM_H