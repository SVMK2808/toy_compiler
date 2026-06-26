#include <stdio.h>
#include "../include/lexer.h"
#include "../include/token.h"
#include "../include/parser.h"
#include "../include/symtable.h"
#include "../include/codegen.h"
#include "../include/vm.h"
#include "../include/ast.h"
#include <stdlib.h>

int main(void){
    Parser p;
       parser_init(&p, 
        "fn fact(n) {\n"
        "    if (n < 2) {\n"
        "        return 1;\n"
        "    }\n"
        "    return n * fact(n - 1);\n"
        "}\n"
        "let result = fact(5);\n"
        "print result;\n"
    );

    int count = 0;
    ASTNode **stmts = parse_program(&p, &count);

    printf("AST: \n");
    for(int i = 0; i < count; i++){
        print_ast(stmts[i], 0);
        printf("\n");
    }

    // codegen
    VM vm;
    vm_init(&vm);
    for(int i = 0; i < count; i++){
        codegen(stmts[i], &vm, false);
    }
    vm_emit(&vm, OP_HALT, 0, NULL);

    //run
    printf("OUTPUT: \n");
    vm_run(&vm);

    // free 
    for(int i = 0; i < count; i ++){
        free_ast(stmts[i]);
    }
    free(stmts);

    return 0;
    
}