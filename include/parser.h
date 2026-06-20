#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include "symtable.h"

#define MAX_STATEMENTS 256
typedef struct {
    Lexer       lexer; // The lexical analyser in the parser
    Token       current; // The token we are currently looking at
    SymTable    symtable; // Symbol table for storing the variables
}Parser;

void    parser_init(Parser *p, const char *src);
ASTNode* parse_expr(Parser *p); //handles + and -
ASTNode* parse_term(Parser *p); //handles * and /
ASTNode* parse_factor(Parser *p); //handles numbers and parentheses
ASTNode* parse_statement(Parser *p);
ASTNode** parse_program(Parser *p, int *count);

#endif //PARSER_H
