#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/symtable.h"

void parser_init(Parser *p, const char *src){
    lexer_init(&p->lexer, src);     // initialise the lexical parser
    symtable_init(&p -> symtable); // initialise the symbol table
    p->current = lexer_next(&p->lexer); //load first token to p->current
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

    if(p -> current.type == TOKEN_IDENTIFIER){
        if(!symtable_exists(&p -> symtable, p -> current.name)){
            printf("Error: Undefined variable: (%s)\n", p -> current.name);
            exit(1);
        }

        char name[64];
        strncpy(name, p -> current.name, 64);
        advance(p);
        return make_ident(name);
    }

    if(p-> current.type == TOKEN_LPAREN){
        advance(p);
        ASTNode *node = parse_expr(p);
        if(p -> current.type != TOKEN_RPAREN){
            fprintf(stderr, "Error: missing closing ')'\n");
            exit(1);
        }
        advance(p);
        return node;
    }

    printf("Error: unexpected token\n");
    exit(1);
}

ASTNode *parse_statement(Parser *p){
    
    // let x = <expr>
    if(p -> current.type == TOKEN_LET){
        advance(p); // consume "let"
        if(p -> current.type != TOKEN_IDENTIFIER){
            printf("Error: expected variable name after 'let'\n");
            exit(1);
        }

        char name[64];
        strncpy(name, p -> current.name, 64);
        advance(p);

        if(p -> current.type != TOKEN_ASSIGN){
            printf("Error: expected '=' after variable  name\n");
            exit(1);
        }

        advance(p);
        ASTNode *value = parse_expr(p);
        symtable_set(&p -> symtable, name, 0);
        return make_let(name, value);
    }

    // print <expr>
    if(p -> current.type == TOKEN_PRINT){
        advance(p);
        ASTNode *expr = parse_expr(p);
        return make_print(expr);
    }

    // plain expression statement
    return parse_expr(p);
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

ASTNode **parse_program(Parser *p, int *count){
    ASTNode **stmts = malloc(sizeof(ASTNode*) * MAX_STATEMENTS);
    *count = 0;
    
    while(p -> current.type != TOKEN_EOF){
        stmts[(*count)++] = parse_statement(p);
        if(p -> current.type == TOKEN_SEMICOLON){
            advance(p); //consume ';'
        }
    }
    return stmts;
}