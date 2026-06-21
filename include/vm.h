#ifndef VM_H
#define VM_H

#include "symtable.h"
#define MAX_STACK 256
#define MAX_CODE  1024

typedef enum {
    OP_PUSH,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_STORE, //pop stack, store into the symbol table
    OP_LOAD,  //push from symtable onto stack 
    OP_PRINT, // pop stack, print value
    OP_HALT
} OpCode;

typedef struct{
    OpCode code;
    double operand; //Only used for OP_PUSH
    char   name[64]; // used for OP_StORE and OP_LOAD
} Instruction;

typedef struct{
    Instruction code[MAX_CODE];   //bytecode instructions
    int         code_count;       //how many instructions

    double      stack[MAX_STACK]; //the stack
    int         stack_top;        //current stack pointer
    
    SymTable    symtable;         // variables live here
} VM;

void vm_init(VM *vm);
void vm_emit(VM *vm, OpCode op, double operand, const char* name);
void vm_run(VM *vm);

#endif //VM_H