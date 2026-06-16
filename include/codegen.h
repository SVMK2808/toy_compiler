#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "vm.h"

void codegen(ASTNode *node, VM *vm);

#endif //CODEGEN_H