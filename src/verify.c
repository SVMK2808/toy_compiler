#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/verify.h"
#include "../include/symtable.h"

void print_smt_bool(FILE *out, ASTNode *node);
void print_smt_expr(FILE *out, ASTNode *node);

// AST cloning 
ASTNode *clone_ast(ASTNode *node){
    if(!node) return NULL;
    ASTNode *copy = malloc(sizeof(ASTNode));
    copy -> type = node -> type;
    switch(node -> type){
        case NODE_NUMBER:
            copy -> number = node -> number;
            break;
        
        case NODE_IDENT:
            strcpy(copy -> ident, node -> ident);
            break;
        
        case NODE_BINOP:
            copy -> binop.op = node -> binop.op;
            copy -> binop.left = clone_ast(node -> binop.left);
            copy -> binop.right = clone_ast(node -> binop.right);
            break;
        
        case NODE_LOGICAL:
            strcpy(copy -> logical.op, node -> logical.op);
            copy -> logical.left = clone_ast(node -> logical.left);
            copy -> logical.right = clone_ast(node -> logical.right);
            break;

        case NODE_UNARY:
            copy -> unary.op = node -> unary.op;
            copy -> unary.operand = clone_ast(node -> unary.operand);
            break;
        
        case NODE_COMPARE:
            strcpy(copy -> compare.op, node -> compare.op);
            copy -> compare.left = clone_ast(node -> compare.left);
            copy -> compare.right = clone_ast(node -> compare.right);
            break;

        default:
            break;
    }

    return copy;
}

// AST Substitution 
ASTNode *substitute(ASTNode *node, const char *var_name, ASTNode *replacement){
    if(!node) return NULL;
    if(node -> type == NODE_IDENT && strcmp(node -> ident, var_name) == 0){
        return clone_ast(replacement);
    }

    ASTNode *copy = malloc(sizeof(ASTNode));
    copy -> type = node -> type;
    switch(node -> type){
        case NODE_NUMBER:
            copy -> number = node -> number;
            break;
        
        case NODE_IDENT:
            strcpy(copy -> ident, node -> ident);
            break;
        
        case NODE_BINOP:
            copy -> binop.op = node -> binop.op;
            copy -> binop.left = substitute(node -> binop.left, var_name, replacement);
            copy -> binop.right = substitute(node -> binop.right, var_name, replacement);
            break;

        case NODE_LOGICAL:
            strcpy(copy -> logical.op, node -> logical.op);
            copy -> logical.left = substitute(node -> logical.left, var_name, replacement);
            copy -> logical.right = substitute(node -> logical.right, var_name, replacement);
            break;

        case NODE_UNARY:
            copy -> unary.op = node -> unary.op;
            copy -> unary.operand = substitute(node -> unary.operand, var_name, replacement);
            break;

        case NODE_COMPARE:
            strcpy(copy -> compare.op, node -> compare.op);
            copy -> compare.left = substitute(node -> compare.left, var_name, replacement);
            copy -> compare.right = substitute(node -> compare.right, var_name, replacement);
            break;

        default:
            break;
    }
    return copy;
}

// Variable Collection
void collect_vars(ASTNode *node, SymTable *vars){
    if(!node) return;
    if(node -> type == NODE_IDENT){
        if(!symtable_exists(vars, node -> ident)){
            symtable_set(vars, node -> ident, 0.0);
        }

        return;
    }

    switch(node -> type){
        case NODE_BINOP:
            collect_vars(node -> binop.left, vars);
            collect_vars(node -> binop.right, vars);
            break;
        
        case NODE_COMPARE:
            collect_vars(node -> compare.left, vars);
            collect_vars(node -> compare.right, vars);
            break;

        case NODE_LOGICAL:
            collect_vars(node -> logical.left, vars);
            collect_vars(node -> logical.right, vars);
            break;

        case NODE_UNARY:
            collect_vars(node -> unary.operand, vars);
            break;
        
        default:
            break;
    }
}

//Check it an ASTNode translates to an SMT bool sort
bool is_smt_bool(ASTNode *node){
    if(!node) return false;
    if(node -> type == NODE_COMPARE || node -> type == NODE_LOGICAL) return true;
    if(node -> type == NODE_UNARY && node -> unary.op == '!') return true;
    return false;
}


// Print SMT - LIB2 S - expression
void print_smt_expr(FILE *out, ASTNode *node){
    if(!node) return;
    
    switch(node -> type){
        case NODE_NUMBER:
            fprintf(out, "%.02f", node -> number);
            break;

        case NODE_IDENT:
            fprintf(out, "%s", node -> ident);
            break;

        case NODE_BINOP:
            fprintf(out, "(%c ", node -> binop.op);
            print_smt_expr(out, node -> binop.left);
            fprintf(out, " ");
            print_smt_expr(out, node -> binop.right);
            fprintf(out, ")");
            break;

        case NODE_COMPARE:
            fprintf(out, "(%s ", node -> compare.op);
            print_smt_expr(out, node -> compare.left);
            fprintf(out, " ");
            print_smt_expr(out, node -> compare.right);
            fprintf(out, ")");
            break;

        case NODE_LOGICAL:
            if(strcmp(node -> logical.op, "&&") == 0){
                fprintf(out, "(and ");
            } else {
                fprintf(out, "(or ");
            }
            print_smt_bool(out, node -> logical.left);
            fprintf(out, " ");
            print_smt_bool(out, node -> logical.right);
            fprintf(out, ")");
            break;

        case NODE_UNARY:
            if(node -> unary.op == '!'){
                fprintf(out, "(not ");
                print_smt_bool(out, node -> unary.operand);
                fprintf(out, ")");
            } else if(node -> unary.op == '-'){
                // SMT-LIB2 unary negation is (-0 x) or (-x)
                fprintf(out, "(- 0.0 ");
                print_smt_expr(out, node -> unary.operand);
                fprintf(out, ")");
            }

            break;

        default:
            break;
    }
}

// Print node wrapped in a coercium to Bool if it is Real
void print_smt_bool(FILE *out, ASTNode *node){
    if(!node) return;
    if(is_smt_bool(node)){
        print_smt_expr(out, node);
    }else {
        fprintf(out, "(not (= ");
        print_smt_expr(out, node);
        fprintf(out, " 0.0))");
    }

}


static bool verification_success = true;

// Call Z3 to prove formula (checks if negation is unsat)
bool prove_formula(ASTNode *vc, const char* check_name){
    if(!vc) return true;

    // Collect all free variables directly from the VC
    SymTable vars;
    symtable_init(&vars);
    collect_vars(vc, &vars);

    // Write to a temporary file
    FILE *out = fopen("temp_query.smt2", "w");
    if(!out){
        fprintf(stderr, "Error: Could not open temp_query.smt2 for writing. \n");
        return false;
    }

    // Declare variables as Reals
    for(int i = 0; i < vars.count; i++){
        fprintf(out, "(declare-const %s Real)\n", vars.symbols[i].name);
    }

    // Assert the negated formula (negation of the VC boolean)
    fprintf(out, "(assert (not ");
    print_smt_bool(out, vc);
    fprintf(out, "))\n");
    fprintf(out, "(check-sat)\n");
    fclose(out);

    // Run Z3
    FILE *fp = popen("z3 temp_query.smt2 2>/dev/null", "r");
    if(!fp){
        fprintf(stderr, "Error: Z3 solver could not be executed. Is it on your PATH?\n");
        remove("temp_query.smt2");
        return false;
    }

    char response[128] = "";
    if(fgets(response, sizeof(response), fp) == NULL){
        fprintf(stderr, "Error: Empty response from Z3 solver.\n");
        pclose(fp);
        remove("temp_query.smt2");
        return false;
    }

    pclose(fp);
    remove("temp_query.smt2");

    // If negation is unsatisfiable, then the original VC holds!
    if(strncmp(response, "unsat", 5) == 0){
        printf("Verification PASS: %s\n", check_name);
        return true;
    }else {
        printf("Verification FAIL: %s (Z3 output: %s)\n", check_name, response);
        verification_success = false;
        return false;
    }
}


    // Forward declarations for Weakest Precondition 
    ASTNode *compute_wp(ASTNode *stmt, ASTNode *post);
    ASTNode *compute_wp_block(ASTNode **stmts, int count, ASTNode *post);

    ASTNode* compute_wp_block(ASTNode **stmts, int count, ASTNode *post){
        ASTNode *curr = clone_ast(post);
        for(int i = count - 1; i >= 0; i--){
            ASTNode *prev = curr;
            curr = compute_wp(stmts[i], curr);
            free_ast(prev);
        }
        return curr;
    }

ASTNode* compute_wp(ASTNode *stmt, ASTNode *post){
    if(!stmt) return clone_ast(post);

    switch(stmt -> type){
        case NODE_LET:
            return substitute(post, stmt -> let.name, stmt -> let.value);

        case NODE_ASSIGN:
            return substitute(post, stmt -> assign.name, stmt -> assign.value);

        case NODE_ASSERT:
            return make_logical("&&", clone_ast(stmt -> assertion.condition), clone_ast(post));

        case NODE_RETURN:
            return substitute(post, "result", stmt -> ret_val);

        case NODE_PRINT:
            return clone_ast(post); // skip

        case NODE_IF: {
            ASTNode *wp_then = compute_wp_block(stmt->if_else.then_body, stmt->if_else.then_count, post);
            ASTNode *wp_else = NULL;
            if (stmt->if_else.else_body) {
                wp_else = compute_wp_block(stmt->if_else.else_body, stmt->if_else.else_count, post);
            } else {
                wp_else = clone_ast(post);
            }

            ASTNode *cond_clone1 = clone_ast(stmt->if_else.condition);
            ASTNode *not_cond = make_unary('!', cond_clone1);
            ASTNode *then_branch = make_logical("||", not_cond, wp_then);
            
            ASTNode *cond_clone2 = clone_ast(stmt->if_else.condition);
            ASTNode *else_branch = make_logical("||", cond_clone2, wp_else);
            
            ASTNode *result = make_logical("&&", then_branch, else_branch);
            return result;
        }

         case NODE_WHILE: {
            ASTNode *inv = stmt->while_loop.invariant;
            ASTNode *cond = stmt->while_loop.condition;
            if (!inv) {
                fprintf(stderr, "Warning: while loop lacks invariant. Static verification might fail.\n");
                return clone_ast(post);
            }

            // 1. Loop Preservation: inv && cond => WP(body, inv)
            ASTNode *wp_body = compute_wp_block(stmt->while_loop.body, stmt->while_loop.body_count, inv);
            ASTNode *inv_and_cond = make_logical("&&", clone_ast(inv), clone_ast(cond));
            ASTNode *not_inv_and_cond = make_unary('!', inv_and_cond);
            ASTNode *preservation_vc = make_logical("||", not_inv_and_cond, wp_body);

            prove_formula(preservation_vc, "While loop preservation");
            free_ast(preservation_vc);

            // 2. Loop Exit: inv && !cond => post
            ASTNode *not_cond = make_unary('!', clone_ast(cond));
            ASTNode *inv_and_not_cond = make_logical("&&", clone_ast(inv), not_cond);
            ASTNode *not_inv_and_not_cond = make_unary('!', inv_and_not_cond);
            ASTNode *exit_vc = make_logical("||", not_inv_and_not_cond, clone_ast(post));

            prove_formula(exit_vc, "while loop exit establishes postcondition");
            free_ast(exit_vc);

            return clone_ast(inv);
         }

         case NODE_FOR: {
            ASTNode *inv = stmt->for_loop.invariant;
            ASTNode *cond = stmt->for_loop.condition;
            ASTNode *incr = stmt->for_loop.increment;
            ASTNode *init = stmt->for_loop.init;
            if (!inv) {
                fprintf(stderr, "Warning: for loop lacks invariant. Static verification might fail.\n");
                return clone_ast(post);
            }

            // 1. Loop Preservation: inv && cond => WP(body; incr, inv)
            ASTNode *wp_incr = compute_wp(incr, inv);
            ASTNode *wp_body = compute_wp_block(stmt->for_loop.body, stmt->for_loop.body_count, wp_incr);
            free_ast(wp_incr);

            ASTNode *inv_and_cond = make_logical("&&", clone_ast(inv), clone_ast(cond));
            ASTNode *not_inv_and_cond = make_unary('!', inv_and_cond);
            ASTNode *preservation_vc = make_logical("||", not_inv_and_cond, wp_body);

            prove_formula(preservation_vc, "For loop preservation");
            free_ast(preservation_vc);

            // 2. Loop Exit: inv && !cond => post
            ASTNode *not_cond = make_unary('!', clone_ast(cond));
            ASTNode *inv_and_not_cond = make_logical("&&", clone_ast(inv), not_cond);
            ASTNode *not_inv_and_not_cond = make_unary('!', inv_and_not_cond);
            ASTNode *exit_vc = make_logical("||", not_inv_and_not_cond, clone_ast(post));

            prove_formula(exit_vc, "For loop exit establishes postcondition.");
            free_ast(exit_vc);

            return compute_wp(init, clone_ast(inv));
         }

         case NODE_DO_WHILE: {
            ASTNode *inv = stmt->do_while.invariant;
            ASTNode *cond = stmt->do_while.condition;
            if (!inv) {
                fprintf(stderr, "Warning: do-while loop lacks invariant. Static verification might fail.\n");
                return clone_ast(post);
            }

            // 1. Loop Preservation: inv => WP(body, inv)
            ASTNode *wp_body = compute_wp_block(stmt->do_while.body, stmt->do_while.body_count, inv);
            ASTNode *not_inv = make_unary('!', clone_ast(inv));
            ASTNode *preservation_vc = make_logical("||", not_inv, wp_body);

            prove_formula(preservation_vc, "Do-while loop preservation.");
            free_ast(preservation_vc);

            // 2. Loop Exit: inv && !cond => post
            ASTNode *not_cond = make_unary('!', clone_ast(cond));
            ASTNode *inv_and_not_cond = make_logical("&&", clone_ast(inv), not_cond);
            ASTNode *not_inv_and_not_cond = make_unary('!', inv_and_not_cond);
            ASTNode *exit_vc = make_logical("||", not_inv_and_not_cond, clone_ast(post));

            prove_formula(exit_vc, "Do-while loop exit establishes postcondition");
            free_ast(exit_vc);

            return clone_ast(inv);
         }

         default:
            return clone_ast(post);
    }
}

// Main static verification entry point
bool verify_program(ASTNode **stmts, int count){
    verification_success = true;

    // 1. Modular verification of function definitions
    for(int i = 0; i < count; i++){
        if(stmts[i]->type == NODE_FUNC_DEF){
            ASTNode *pre = stmts[i] -> func_def.precondition;
            ASTNode *post = stmts[i] -> func_def.postcondition;

            // Default pre/post to 1.0 (True) if NULL
            ASTNode *actual_pre = pre ? pre : make_number(1.0);
            ASTNode *actual_post = post ? post : make_number(1.0);

            ASTNode *wp = compute_wp_block(stmts[i] -> func_def.body, stmts[i] -> func_def.body_count, actual_post);

            // Verify: actual_pre => wp (i.e. !actual_pre || wp)
            ASTNode *not_pre = make_unary('!', clone_ast(actual_pre));
            ASTNode *vc = make_logical("||", not_pre, wp);

            char desc[128];
            snprintf(desc, sizeof(desc), "Function contract for '%s'", stmts[i] -> func_def.name);
            prove_formula(vc, desc);

            free_ast(vc); // Properly frees not_pre and wp recursively
            if (!pre) free_ast(actual_pre);
            if (!post) free_ast(actual_post);
        }
    }

    // 2. Verifcation of top - level code statments
    int tl_count = 0;
    ASTNode **tl_stmts = malloc(sizeof(ASTNode*) * count);
    for(int i = 0; i < count; i++){
        if(stmts[i] -> type != NODE_FUNC_DEF){
            tl_stmts[tl_count++] = stmts[i];
        }
    }

    if(tl_count > 0){
        ASTNode *post = make_number(1.0);
        ASTNode *wp = compute_wp_block(tl_stmts, tl_count, post);

        prove_formula(wp, "Main program assertions");

        free_ast(post);
        free_ast(wp);
    }
    free(tl_stmts);

    return verification_success;
}



