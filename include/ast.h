#ifndef AST_H
#define AST_H

typedef enum{
    NODE_NUMBER, //a literal number eg. 42
    NODE_BINOP, // a binary operation eg. +, -, *,/
    NODE_IDENT, // variable reference eg. x
    NODE_LET,   // variable declaration eg. let x = 5
    NODE_PRINT,  // print statement
    NODE_IF,    //if/else statement
    NODE_COMPARE //comparision >, <, ==

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

    struct{
        char            op[3];      // '>', '<', '>=', '<=', '=='
        struct ASTNode *left;
        struct ASTNode *right;
    }compare;                       // for NODE_COMPARE

    struct {
        struct ASTNode *condition;  //the if condition
        struct ASTNode **then_body;  // the true branch
        int             then_count;  // to count number of statements in the true branch
        struct ASTNode **else_body;  // else branch (can be NULL)
        int             else_count;  // to count number of statements in the false branch
    } if_else;                      // for NODE_IF

    struct ASTNode *print;      // for NODE_PRINT - expression to print
    
    };
}ASTNode;

ASTNode* make_number(double value);
ASTNode* make_binop(char op, ASTNode* left, ASTNode* right);
ASTNode* make_let(const char *name,ASTNode *value);
ASTNode* make_print(ASTNode* expr);
ASTNode* make_ident(const char* name);
ASTNode* make_compare(char *op, ASTNode *left, ASTNode *right);
ASTNode* make_if(ASTNode *condition, 
                 ASTNode **then_body, int then_count,
                 ASTNode **else_body, int else_count);
void     print_ast(ASTNode* node, int depth);
void     free_ast(ASTNode* node);





#endif //AST_H