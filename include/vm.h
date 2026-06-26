#ifndef VM_H
#define VM_H

#include "symtable.h"
#define MAX_STACK 256
#define MAX_CODE  1024
#define MAX_PARAMS 8
#define MAX_FUNCS  64
#define MAX_FRAMES 64

typedef enum {
    OP_PUSH,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LT,          // greater than
    OP_GT,          // lesser than
    OP_EQ,          // equal to
    OP_JMP,         // unconditional jump
    OP_JMP_IF_FALSE,// conditional jump
    OP_STORE, //pop stack, store into the symbol table
    OP_LOAD,  //push from symtable onto stack 
    OP_PRINT, // pop stack, print value
    OP_CALL, // call a function by name
    OP_RETURN, // return from function, leave return value on stack
    OP_LOAD_LOCAL, // load from the current call frame's local scope
    OP_STORE_LOCAL, // store into the current call frame's local scope
    OP_HALT
} OpCode;

typedef struct{
    OpCode code;
    double operand; //Only used for OP_PUSH
    char   name[64]; // used for OP_StORE and OP_LOAD
} Instruction;

typedef struct{
    char        name[64];
    int         code_start;
    int         param_count;
    char        params[MAX_PARAMS][64];
}FuncDef;

typedef struct{
    FuncDef     funcs[MAX_FUNCS];
    int         count;
}FuncTable; 

typedef struct {
    int         return_addr;  // instruction index to jump back to
    SymTable    locals;      // local variable scope for this call
}CallFrame;


typedef struct{
    Instruction code[MAX_CODE];   //bytecode instructions
    int         code_count;       //how many instructions

    double      stack[MAX_STACK]; //the stack
    int         stack_top;        //current stack pointer
    
    SymTable    symtable;         // variables live here
    FuncTable   func_table;
    CallFrame   frames[MAX_FRAMES];
    int         frame_top;        // -1 means we're at the top level
} VM;


void vm_init(VM *vm);
void vm_emit(VM *vm, OpCode op, double operand, const char* name);
void vm_run(VM *vm);
void vm_register_func(VM *vm, const char* name, int code_start, int param_count, char params[][64]);

#endif //VM_H