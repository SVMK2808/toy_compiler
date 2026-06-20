#include <stdio.h>
#include "../include/lexer.h"
#include "../include/token.h"
#include "../include/parser.h"
#include "../include/symtable.h"
#include "../include/ast.h"
#include <stdlib.h>

int main(void){
    Parser p;
    parser_init(&p, "let x = 5 + 3; let y = x; print y");

    int count = 0;
    ASTNode **stmts = parse_program(&p, &count);

    printf("AST: \n");
    for(int i = 0; i < count; i++){
        print_ast(stmts[i], 0);
        printf("\n");
    }

    // free 
    for(int i = 0; i < count; i ++){
        free_ast(stmts[i]);
    }
    free(stmts);

    return 0;
    
}