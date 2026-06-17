#include <stdio.h>
#include "../include/parser.h"
#include "../include/ast.h"
#include "../include/codegen.h"
#include "../include/vm.h"
#include <stdlib.h>

int main(int argc, char* argv[]){
    if(argc < 2){
        perror("Usage: ./compiler <expression>\n");
        exit(1);
    }
    Parser p;
    parser_init(&p, argv[1]);
    ASTNode *tree = parse_expr(&p);

    //Codegen
    VM vm;
    vm_init(&vm);
    codegen(tree, &vm);
    vm_emit(&vm, OP_HALT, 0);

    //Run
    printf("AST tree:\n");
    print_ast(tree, 0);
    vm_run(&vm);
    
    free_ast(tree);
    return 0;
}