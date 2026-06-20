#ifndef AST_H
#define AST_H

typedef enum{
    NODE_NUMBER, //a literal number eg. 42
    NODE_BINOP, // a binary operation eg. +, -, *,/
    NODE_IDENT, // variable reference eg. x
    NODE_LET,   // variable declaration eg. let x = 5
    NODE_PRINT  // print statement

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

        char ident[64];       // for NODE_IDENT - variable name
    struct {
        char            name[64];   //variable name
        struct ASTNode  *value;     // expression assigned to it
    } let;

    struct ASTNode *print;      // for NODE_PRINT - expression to print
    
    };
}ASTNode;

ASTNode* make_number(double value);
ASTNode* make_binop(char op, ASTNode* left, ASTNode* right);
ASTNode* make_let(const char *name,ASTNode *value);
ASTNode* make_print(ASTNode* expr);
ASTNode* make_ident(const char* name);
void     print_ast(ASTNode* node, int depth);
void     free_ast(ASTNode* node);





#endif //AST_H