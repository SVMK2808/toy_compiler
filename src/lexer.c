#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../include/lexer.h"

void lexer_init(Lexer *l, const char *src){
    l->src = src;
    l->pos = 0;
}

Token lexer_next(Lexer *l){
    //Skip whitespace
    while(l->src[l->pos] != '\0' && isspace(l->src[l->pos]))
        l->pos++;
    
    char c = l->src[l->pos];
    // EOF

    if(c == '\0') return (Token){ TOKEN_EOF, 0};

    // Number 
    if(isdigit(c)){
        double val = 0;
        while(isdigit(l->src[l->pos])){
            val = val * 10 + (l->src[l->pos++] - '0');
        }
        return (Token) {TOKEN_NUMBER, val};
    }

    l -> pos++;

    switch(c){
        case '+' : return (Token) {TOKEN_PLUS, 0};
        case '-' : return (Token) {TOKEN_MINUS, 0};
        case '*' : return (Token) {TOKEN_STAR, 0};
        case '/' : return (Token) {TOKEN_SLASH, 0};
        case '(' : return (Token) {TOKEN_LPAREN, 0};
        case ')' : return (Token) {TOKEN_RPAREN, 0};
        default : return (Token) {TOKEN_UNKNOWN, 0};

    }

}