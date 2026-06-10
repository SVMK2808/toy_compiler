#include <stdlib.h>
#include <stdio.h>
#include "../include/parser.h"

void parser_init(Parser *p, const char *src){
    lexer_init(&p->lexer, src);
    p->current = lexer_next(&p->lexer); //load first token
}

//forward declaration
ASTNode *parse_expr(Parser *p);

static Token advance(Parser *p){
    Token prev = p -> current;
    p -> current = lexer_next(&p-> lexer); //move to the next token
    return prev;
}

ASTNode *parse_factor(Parser *p){
    if(p-> current.type == TOKEN_NUMBER){
        double val = p->current.value;
        advance(p);
        return make_number(val);
    }

    if(p-> current.type == TOKEN_LPAREN){
        advance(p);
        ASTNode *node = parse_expr(p);
        advance(p);
        return node;
    }

    printf("Error: unexpected token\n");
    exit(1);
}

ASTNode *parse_term(Parser *p){
    ASTNode *left = parse_factor(p);  //get left side

    while(p -> current.type == TOKEN_STAR ||
          p-> current.type == TOKEN_SLASH){

            char op = (p -> current.type == TOKEN_STAR) ? '*' : '/';
            advance(p);                            // consume the operator
            ASTNode *right = parse_factor(p);      // get right side
            left = make_binop(op, left, right);    // build node
    }
      return left;
}

ASTNode *parse_expr(Parser *p){
    ASTNode *left = parse_term(p);

    while(p -> current.type == TOKEN_PLUS || 
          p-> current.type == TOKEN_MINUS){
            char op = (p -> current.type == TOKEN_PLUS)? '+' : '-';
            advance(p);
            ASTNode *right = parse_term(p);
            left = make_binop(op, left, right);
          }
    return left;
}