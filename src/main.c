#include <stdio.h>
#include "../include/parser.h"
#include "../include/ast.h"

int main(){
    Parser p;
    parser_init(&p, "1 + 2 * (3 - 4)");

    ASTNode *tree = parse_expr(&p);
    print_ast(tree, 0);
    free_ast(tree);


    
    return 0;
}