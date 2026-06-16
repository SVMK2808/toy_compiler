#include <stdio.h>
#include "../include/parser.h"
#include "../include/ast.h"
#include "../include/codegen.h"
#include "../include/vm.h"

int main(){
    Parser p;
    parser_init(&p, "(1 + 2) * (3 + 4)");
    ASTNode *tree = parse_expr(&p);

    //Codegen
    VM vm;
    vm_init(&vm);
    codegen(tree, &vm);
    vm_emit(&vm, OP_HALT, 0);

    //Run
    printf("AST tree:\n");
    print_ast(tree, 0);
    printf("\nResult: ");
    vm_run(&vm);
    
    free_ast(tree);
    return 0;
}