#ifndef AST_H
#define AST_H

typedef enum{
    NODE_NUMBER, //a literal number eg. 42
    NODE_BINOP, // a binary operation eg. +, -, *,/
    NODE_IDENT, // variable reference eg. x
    NODE_LET,   // variable declaration eg. let x = 5
    NODE_PRINT,  // print statement
    NODE_IF,    //if/else statement
    NODE_COMPARE, //comparision >, <, ==
    NODE_WHILE, // for while loop
    NODE_DO_WHILE, // do{ .... }while(cond)
    NODE_ASSIGN    // for x = expr (reassignment of existing variable)
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

    struct{
        char            name[64];
        struct ASTNode *value;
    }assign; // for NODE_ASSIGN
    struct {
        struct ASTNode *condition;  //the if condition
        struct ASTNode **then_body;  // the true branch
        int             then_count;  // to count number of statements in the true branch
        struct ASTNode **else_body;  // else branch (can be NULL)
        int             else_count;  // to count number of statements in the false branch
    } if_else;          // for NODE_IF

    struct {
        struct ASTNode *condition;  // the loop condition
        struct ASTNode **body;      // statements inside the loop body
        int             body_count; // number of statements in the loop body
    } while_loop;       // for NODE_WHILE

    struct{
        struct ASTNode *condition;  // the loop condition
        struct ASTNode **body;      // statments in the loop body
        int             body_count; // number of statements in the loop body
    }do_while;          // for NODE_DO_WHILE
    struct ASTNode *print;      // for NODE_PRINT - expression to print
    
    };
}ASTNode;

//literals
ASTNode* make_ident(const char* name);
ASTNode* make_number(double value);
// Arithmetic Operators - '-', '+', '*', '/'
ASTNode* make_binop(char op, ASTNode* left, ASTNode* right);

// Keyword 'let'
ASTNode* make_let(const char *name,ASTNode *value);

//Keyword 'print'
ASTNode* make_print(ASTNode* expr);
// Comparision operators - '>', '<', '=='
ASTNode* make_compare(char *op, ASTNode *left, ASTNode *right);
// if - else statement
ASTNode* make_if(ASTNode *condition, 
                 ASTNode **then_body, int then_count,
                 ASTNode **else_body, int else_count);
// For x = <expr> (reassignment of same variable)
ASTNode *make_assign(const char* name, ASTNode* value);
// While loop
ASTNode* make_while(ASTNode *condition, ASTNode **body, int body_count);
// Do-while loop
ASTNode* make_do_while(ASTNode *condition, ASTNode **body, int body_count);
//Print AST
void     print_ast(ASTNode* node, int depth);
//Free the memory used for AST
void     free_ast(ASTNode* node);





#endif //AST_H