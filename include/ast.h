#ifndef AST_H
#define AST_H

typedef enum{
    NODE_NUMBER, //a literal number eg. 42
    NODE_BINOP, // a binary operation eg. +, -, *,/
    NODE_LOGICAL, // for && and || 
    NODE_UNARY, // for ! and unary negation -
    NODE_IDENT, // variable reference eg. x
    NODE_LET,   // variable declaration eg. let x = 5
    NODE_PRINT,  // print statement
    NODE_IF,    //if/else statement
    NODE_COMPARE, //comparision >, <, ==
    NODE_WHILE, // for while loop
    NODE_DO_WHILE, // do{ .... }while(cond)
    NODE_FOR,      // for(init; cond; incr){ body }
    NODE_ASSIGN,    // for x = expr (reassignment of existing variable)
    NODE_FUNC_DEF, // fn name(params) { body }
    NODE_FUNC_CALL, // name(args)
    NODE_RETURN,    // return <expr>
    NODE_ARRAY_LIT, // array literal eg. [1, 2, 3]
    NODE_ARRAY_INDEX, // array index eg. arr[index]
    NODE_ARRAY_ASSIGN, // array index assignment eg. arr[index] = val
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

        struct {
            char            op[3]; // "&&" or "||"
            struct ASTNode* left;
            struct ASTNode* right;
        } logical;            // for NODE_LOGICAL

        struct{
            char            op;  // '!' or '-'
            struct ASTNode* operand; 
        }unary;             // for NODE_UNARY
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

    struct{
        struct ASTNode *init;   // initializer (let i = 0 or i = 0)
        struct ASTNode *condition; // condition for the 'for' loop
        struct ASTNode *increment; // increment (i = i + 1)
        struct ASTNode **body;  // loop body statements
        int            body_count;
    } for_loop;     // for NODE_FOR

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

    struct {
        struct ASTNode **elements; 
        int            element_count;
    } array_lit;        // for NODE_ARRAY_LIT

    struct {
        struct ASTNode *array;
        struct ASTNode *index;
    } array_index;      // for NODE_ARRAY_INDEX

    struct {
        char            name[64];
        struct ASTNode *index;
        struct ASTNode *value;
    }array_assign;      // for NODE_ARRAY_ASSIGN

    struct{
        char           name[64]; // function name
        char           params[8][64]; // 2D array for params
        int            param_count;  // number of parameters
        struct ASTNode **body;  //function definition
        int            body_count; // number of statements in the body
    } func_def;         // for NODE_FUNC_DEF

    struct{
        char           name[64];    // name of the function in the function call
        struct ASTNode **args;      // arguments for func call
        int            arg_count;   // number of arguments in the function
    }func_call;        // for NODE_FUNC_CALL

    struct ASTNode *ret_val;  // for NODE_RETURN
    struct ASTNode *print;      // for NODE_PRINT - expression to print
    
    };
}ASTNode;

//literals
ASTNode* make_ident(const char* name);
ASTNode* make_number(double value);
// Arithmetic Operators - '-', '+', '*', '/'
ASTNode* make_binop(char op, ASTNode* left, ASTNode* right);
// Logical operators - "&&" or "||"
ASTNode* make_logical(const char* op, ASTNode *left, ASTNode *right);
// Unary Operators - '!' or '-'
ASTNode* make_unary(char op, ASTNode *operand);

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
// For loop
ASTNode* make_for(ASTNode *init, ASTNode *condition, ASTNode *incr, ASTNode **body, int body_count);
// For making array literal
ASTNode* make_array_lit(struct ASTNode **elements, int count);
// For making array index eg. arr[index]
ASTNode* make_array_index(struct ASTNode *array, struct ASTNode *index);
// For making array assignment eg. arr[index] = 1
ASTNode* make_array_assign(const char* name, struct ASTNode *index, struct ASTNode *value);
// for function definition
ASTNode* make_func_def(const char *name, char params[][64], int param_count, ASTNode **body, int body_count);
// for function call
ASTNode* make_func_call(const char *name, ASTNode **args, int arg_count);
// for return statment
ASTNode* make_return(ASTNode *value);
//Print AST
void     print_ast(ASTNode* node, int depth);
//Free the memory used for AST
void     free_ast(ASTNode* node);





#endif //AST_H