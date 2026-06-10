#ifndef AST_H
#define AST_H

typedef enum{
    NODE_NUMBER, //a literal number eg. 42
    NODE_BINOP, // a binary operation eg. +, -, *,/
}NodeType;

typedef struct ASTNode{
    NodeType type;

    union {
        double number; // used when type == NODE_NUMBER
        struct{
            char            op;   // '+', '-', '*', '/'
            struct ASTNode* left;
            struct ASTNode* right;
        }binop;           // used when type == NODE_BINOP
    };
}ASTNode;

ASTNode* make_number(double value);
ASTNode* make_binop(char op, ASTNode* left, ASTNode* right);
void     print_ast(ASTNode* node, int depth);
void     free_ast(ASTNode* node);




#endif //AST_H