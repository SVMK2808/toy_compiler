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

static Token peek_next(Parser *p){
    Lexer saved = p -> lexer;
    //snapshot lexer state (just src + pos)
    Token next = lexer_next(&p -> lexer); // read the next token
    p -> lexer = saved;
    // restore, as if we never read it.
    return next;
}

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

    if(p -> current.type == TOKEN_TRUE){
        advance(p);
        return make_number(1.0);
    }

    if(p -> current.type == TOKEN_FALSE){
        advance(p);
        return make_number(0.0);
    }


    if(p -> current.type == TOKEN_IDENTIFIER){
        Token next = peek_next(p);
        if(next.type == TOKEN_LPAREN){
            char name[64];
            strncpy(name, p -> current.name, 64);
            advance(p); // consume identifier
            advance(p); // consume '('

            ASTNode **args = malloc(sizeof(ASTNode*) * 8);
            int arg_count = 0;
            if(p -> current.type != TOKEN_RPAREN){
                args[arg_count++] = parse_expr(p);
                while(p -> current.type == TOKEN_COMMA){
                    advance(p);     // consume ','
                    args[arg_count++] = parse_expr(p);
                }
            }

            if(p -> current.type != TOKEN_RPAREN){
                fprintf(stderr, "Error: expected ')' in function call. \n");
                exit(1);
            }
            advance(p); // consume ')'
            return make_func_call(name, args, arg_count);
        }

        // regular variable reference
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

    printf("Error: unexpected token: \n");
    exit(1);
}

ASTNode **parse_block(Parser *p, int *count){
    ASTNode **stmts = malloc(sizeof(ASTNode*) * MAX_STATEMENTS);
    *count = 0;

    if(p -> current.type != TOKEN_LBRACE){
        printf("Error: expected '{' \n");
        exit(1);
    }
    advance(p);     //consume '{'
    while(p -> current.type != TOKEN_RBRACE && 
          p -> current.type != TOKEN_EOF){
            stmts[(*count)++] = parse_statement(p);
            if(p -> current.type == TOKEN_SEMICOLON)
                advance(p);
          }

          if(p -> current.type != TOKEN_RBRACE){
            printf("Error: expected '}'\n");
            exit(1);
          }

          advance(p);   // consume '}'
          return stmts;
}
ASTNode *parse_statement(Parser *p){
    // fn name(p1, p2) { <body> }
    if(p -> current.type == TOKEN_FN){
        advance(p); // consume 'fn'
        if(p -> current.type != TOKEN_IDENTIFIER){
            printf("Error: expected function name after 'fn'.\n");
            exit(1);
        }

        char name[64];
        strncpy(name, p -> current.name, 64);
        advance(p);

        if(p -> current.type != TOKEN_LPAREN){
            printf("Error: expected '(' after function name.\n");
            exit(1);
        }

        advance(p); // consume '('
        char params[8][64];
        int param_count = 0;
        if(p -> current.type != TOKEN_RPAREN){
            if(p -> current.type != TOKEN_IDENTIFIER){
                printf("Error: expected parameter name.\n");
                exit(1);
            }

            strncpy(params[param_count++], p -> current.name, 64);
            advance(p);

            while(p -> current.type == TOKEN_COMMA){
                advance(p); // consume ','
                if(p -> current.type != TOKEN_IDENTIFIER){
                    printf("Error: expected parameter name after ','\n");
                    exit(1);
                }
                strncpy(params[param_count++], p -> current.name, 64);
                advance(p);
            }
        }

        if(p -> current.type != TOKEN_RPAREN){
            printf("Error: expected ')' after parameters. \n");
            exit(1);
        }
        advance(p);  // consume ')'

        // Backup symtable for local scope of function parameters/locals
        SymTable saved_table = p -> symtable;

        // Register parameters in local parser symtable 
        for(int i = 0; i < param_count; i++){
            symtable_set(&p -> symtable, params[i], 0);
        }
        int body_count = 0;
        ASTNode **body = parse_block(p, &body_count);

        // Restore symtable (discarding local parameters and variables)
        p -> symtable = saved_table;
        return make_func_def(name, params, param_count, body, body_count);
    }

    // return <expr>
    if(p -> current.type == TOKEN_RETURN){
        advance(p); // consume 'return' 
        ASTNode *ret_val = parse_expr(p);
        return make_return(ret_val);
    }
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

    // if ( <condition> ) { <then> } else { <else> }
    if(p -> current.type == TOKEN_IF){
        advance(p);   // consume 'if'
        if(p -> current.type != TOKEN_LPAREN){
            printf("Error: expected '(' after 'if' \n");
            exit(1);
        }

        advance(p);     // consume '('

        // parse condition
        ASTNode *condition = parse_expr(p);

        
        if(p -> current.type != TOKEN_RPAREN){
            printf("Error: expected ')' \n");
            exit(1);
        }

        advance(p);  // consume ')'
        
        // parse then block 
        int then_count = 0;
        ASTNode **then_body = parse_block(p, &then_count);

        // parse optional else block 
        int else_count = 0;
        ASTNode **else_body = NULL;
        if(p -> current.type == TOKEN_ELSE){
            advance(p);     //consume 'else'
            else_body = parse_block(p, &else_count);
        }

        return make_if(condition, then_body, then_count,
                        else_body, else_count);

    }

    // while ( <condition> ) { <body> }
    if(p -> current.type == TOKEN_WHILE){
        advance(p);     // consume 'while'
        if(p -> current.type != TOKEN_LPAREN){
            printf("Error: expected a '(' after 'while'. \n");
            exit(1);
        }

        advance(p); // consume '('
        ASTNode *condition = parse_expr(p);
        if(p -> current.type != TOKEN_RPAREN){
            printf("Error: expected ')' after while condition. \n");
            exit(1);
        }
        advance(p);

        int body_count = 0;
        ASTNode **body = parse_block(p, &body_count);
        return make_while(condition, body, body_count);

    }

    // do { <body> } while (<cpndition> )
    if(p -> current.type == TOKEN_DO){
        advance(p); // consume 'do'
        if(p -> current.type != TOKEN_LBRACE){
            printf("Error: expected a '{' after 'do'.\n");
            exit(1);
        }

        int count = 0;
        ASTNode **body = parse_block(p, &count); // parse { ... }

        if(p -> current.type != TOKEN_WHILE){
            printf("Error: expected 'while' after do block. \n");
            exit(1);
        }

        advance(p); // consume 'while'
        if(p -> current.type != TOKEN_LPAREN){
            printf("Error: expected a '(' after 'while' in do-while.\n");
            exit(1);
        }

        advance(p); // consume '('
        
        ASTNode *condition = parse_expr(p);
        if(p -> current.type != TOKEN_RPAREN){
            printf("Error: expected ')' after do-while condition. \n");
            exit(1);
        }

        advance(p);
        return make_do_while(condition, body, count);

    }

    // variable reassignment: x = expr
    if(p -> current.type == TOKEN_IDENTIFIER){
        Token next = peek_next(p);
        if(next.type == TOKEN_ASSIGN){
            char name[64];
            strncpy(name, p -> current.name, 64);
            advance(p); // consume identifier
            advance(p); // consume '='

            if(!symtable_exists(&p -> symtable, name)){
                printf("Error: undefined variable '%s'", name);
                exit(1);
            }

            ASTNode *value = parse_expr(p);
            return make_assign(name, value);
        }
    }

    // for (init; cond; incr) { body }
    if(p -> current.type == TOKEN_FOR){
        advance(p); // consume 'for'
        if(p -> current.type != TOKEN_LPAREN){
            printf("Error: expected '(' after 'for'. \n");
            exit(1);
        }
        advance(p); // consume '('
        // 1. initializer: let i = 0 or i = 0
        ASTNode *init = parse_statement(p);
        if(p -> current.type != TOKEN_SEMICOLON){
            printf("Error: expected ';' after for initializer.\n");
            exit(1);
        }
        advance(p);     // consume ';' 
        // 2. condition: i < 10
        ASTNode *condition = parse_expr(p);
        if(p -> current.type != TOKEN_SEMICOLON){
            printf("Error: expected ';' after for initializer.\n");
            exit(1);
        }
        advance(p);     // consume ';'
        
        //3. increment i = i + 1
        ASTNode *incr = parse_statement(p);
        if(p -> current.type != TOKEN_RPAREN){
            printf("Error: expected ')' after for increment.\n");
            exit(1);
        }
        advance(p);     // consume ')'

        // 4. body
        int body_count = 0;
        ASTNode **body = parse_block(p, &body_count);

        return make_for(init, condition, incr, body, body_count);

        
    }

    
    // plain expression statement
    return parse_expr(p);
}

ASTNode *parse_unary(Parser *p){
    if(p -> current.type == TOKEN_MINUS){
        advance(p);     // consume the '-'
        ASTNode *operand = parse_unary(p); // right - association
        return make_unary('-', operand);
    }

    if(p -> current.type == TOKEN_NOT){
        advance(p);   // consume the '!'
        ASTNode *operand = parse_unary(p); // right - associative
        return make_unary('!', operand);
    }

    return parse_factor(p);
}

ASTNode *parse_term(Parser *p){
    ASTNode *left = parse_unary(p);  //get left side

    while(p -> current.type == TOKEN_STAR ||
          p-> current.type == TOKEN_SLASH){

            char op = (p -> current.type == TOKEN_STAR) ? '*' : '/';
            advance(p);                            // consume the operator
            ASTNode *right = parse_unary(p);      // get right side
            left = make_binop(op, left, right);    // build node
    }
      return left;
}


ASTNode *parse_add_sub(Parser *p){
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

ASTNode *parse_comparison(Parser *p){
    ASTNode *left = parse_add_sub(p);
    while(p -> current.type == TOKEN_LT || 
          p -> current.type == TOKEN_GT ||
          p -> current.type == TOKEN_LE ||
          p -> current.type == TOKEN_GE){
            char op[3];
            if(p -> current.type == TOKEN_LT) strncpy(op, "<", 3);
            else if(p -> current.type == TOKEN_GT) strncpy(op, ">", 3);
            else if(p -> current.type == TOKEN_LE) strncpy(op, "<=", 3);
            else if(p -> current.type == TOKEN_GE) strncpy(op, ">=", 3);

            advance(p);
            ASTNode *right = parse_add_sub(p);
            left = make_compare(op, left, right);
          }
          return left;
}

ASTNode *parse_equality(Parser *p){
    ASTNode *left = parse_comparison(p);
    while(p -> current.type == TOKEN_EQ){
        advance(p); // consume '='
        ASTNode *right = parse_comparison(p);
        left = make_compare("==", left, right);
    }
    return left;
}

ASTNode *parse_logical_and(Parser *p){
    ASTNode *left = parse_equality(p);
    while(p -> current.type == TOKEN_AND){
        advance(p);     // consume &&
        ASTNode *right = parse_equality(p);
        left = make_logical("&&", left, right);
    }
    return left;
}

ASTNode *parse_expr(Parser *p){
    ASTNode *left = parse_logical_and(p);
    while(p -> current.type == TOKEN_OR){
        advance(p);         // consume ||
        ASTNode *right = parse_logical_and(p);
        left = make_logical("||", left, right);
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