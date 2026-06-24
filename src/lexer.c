#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../include/lexer.h"

static Token make_token(TokenType token){
    Token t;
    t.type = token;
    t.name[0] = '\0';
    t.value = 0.0;
    return t;
}

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

    if(c == '\0') return make_token(TOKEN_EOF);

    //identifiers and keywords
    if(isalpha(c) || c == '_'){
        char buf[64];
        int len = 0;
        while(isalnum(l->src[l->pos]) || l -> src[l -> pos] == '_'){
            buf[len++] = l -> src[l->pos++];
        }
        buf[len] = '\0';
        
        //check for keywords
        Token t = make_token(TOKEN_IDENTIFIER);
        strncpy(t.name, buf, 64);
        if(strcmp(t.name, "let") == 0) t.type = TOKEN_LET;
        else if(strcmp(t.name, "if") == 0) t.type = TOKEN_IF;
        else if(strcmp(t.name, "else") == 0) t.type = TOKEN_ELSE;
        else if(strcmp(t.name, "while") == 0) t.type = TOKEN_WHILE;
        else if(strcmp(t.name, "print") == 0) t.type = TOKEN_PRINT;
        else if(strcmp(t.name, "do") == 0) t.type = TOKEN_DO;

        return t;
    }

    // Number 
    if(isdigit(c)){
        double val = 0;
        while(isdigit(l->src[l->pos])){
            val = val * 10 + (l->src[l->pos++] - '0');
        }
        Token t = make_token(TOKEN_NUMBER);
        t.value = val;

        return t;
    }

    l -> pos++;

    //two character operators
    if(c == '=' && l -> src[l-> pos] == '='){
        l->pos++;
        return make_token(TOKEN_EQ);
    }

    //one character operators and punchuation
    switch(c){
        case '+' : return make_token(TOKEN_PLUS);
        case '-' : return make_token(TOKEN_MINUS);
        case '*' : return make_token(TOKEN_STAR);
        case '/' : return make_token(TOKEN_SLASH);
        case '=' : return make_token(TOKEN_ASSIGN);
        case '<' : return make_token(TOKEN_LT);
        case '>' : return make_token(TOKEN_GT);
        case '(' : return make_token(TOKEN_LPAREN);
        case ')' : return make_token(TOKEN_RPAREN);
        case '{' : return make_token(TOKEN_LBRACE);
        case '}' : return make_token(TOKEN_RBRACE);
        case ';' : return make_token(TOKEN_SEMICOLON);
        default : return make_token(TOKEN_UNKNOWN);

    }

}