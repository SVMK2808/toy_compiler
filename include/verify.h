#ifndef VERIFY_H
#define VERIFY_H

#include "ast.h"
#include <stdbool.h>

bool verify_program(ASTNode **stmts, int count);

#endif