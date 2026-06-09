#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // Literals
    TOKEN_NUMBER,

    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,

    // Parentheses
    TOKEN_LPAREN,
    TOKEN_RPAREN,

    // Special
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    double    value;  // only used when type == TOKEN_NUMBER
} Token;

static inline const char *token_type_to_str(TokenType type){
    switch(type){
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_EOF: return "EOF";
        case TOKEN_UNKNOWN: return "UNKOWN";
        default: return "???";
    }
}

#endif /* TOKEN_H */