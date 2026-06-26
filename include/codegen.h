#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>
#include "ast.h"
#include "vm.h"

void codegen(ASTNode *node, VM *vm, bool in_func);

#endif //CODEGEN_H