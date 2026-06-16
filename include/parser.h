#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer lexer; // The lexical analyser in the parser
    Token current; // The token we are currently looking at
}Parser;

void    parser_init(Parser *p, const char *src);
ASTNode* parse_expr(Parser *p); //handles + and -
ASTNode* parse_term(Parser *p); //handles * and /
ASTNode* parse_factor(Parser *p); //handles numbers and parentheses

#endif //PARSER_H
