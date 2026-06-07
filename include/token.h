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

#endif /* TOKEN_H */