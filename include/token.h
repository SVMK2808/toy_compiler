#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // Literals
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER, // for the variables like x, y, myVar, etc

    //Keywords
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_PRINT,
    TOKEN_DO,

    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_ASSIGN, // =
    TOKEN_LT, // <
    TOKEN_GT, // >
    TOKEN_EQ, // ==

    // Punctuation
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE, // {
    TOKEN_RBRACE, // }
    TOKEN_SEMICOLON, // ;
    

    // Special
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    double    value;  // only used when type == TOKEN_NUMBER
    char      name[64]; // used when type == TOKEN_IDENTIFIER
} Token;

static inline const char *token_type_to_str(TokenType type){
    switch(type){
        //Literals
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";

        //Keywords
        case TOKEN_LET: return "LET";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_PRINT: return "PRINT";
        case TOKEN_DO: return "DO";

        //Operators
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_EQ: return "EQ";

        //Punctuation
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_SEMICOLON: return "SEMICOLON";

        //Special characters
        case TOKEN_EOF: return "EOF";
        case TOKEN_UNKNOWN: return "UNKOWN";
        default: return "???";
    }
}

#endif /* TOKEN_H */