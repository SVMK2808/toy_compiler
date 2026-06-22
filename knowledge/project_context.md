# Toy Compiler — Project Knowledge Base

## Overview

A **daily compiler-building project** by `svmk`. Each day adds a new layer to a toy compiler written in **C**, starting from scratch. As of **Day 9**, the compiler has a full end-to-end pipeline.

- **Language:** C (compiled with `gcc -Wall -Wextra -Iinclude`)
- **Build:** `make` → produces `./compiler` binary
- **Day 9 reference:** https://claude.ai/share/e5f6ca38-b7b8-48f0-896b-5b83b78caf6d

---

## Architecture

```
Source string
     │
     ▼
 [Lexer]       src/lexer.c, include/lexer.h
   Tokenises input into a stream of Token structs
     │
     ▼
 [Parser]      src/parser.c, include/parser.h
   Recursive-descent parser. Builds an AST.
   Holds its own SymTable to validate declared variables at parse time.
     │
     ▼
 [AST]         src/ast.c, include/ast.h
   Tree of ASTNode structs (union-based, discriminated by NodeType)
     │
     ▼
 [Codegen]     src/codegen.c, include/codegen.h
   Walks the AST and emits bytecode instructions into a VM struct
     │
     ▼
 [VM]          src/vm.c, include/vm.h
   Stack-based virtual machine that executes bytecode
   Has its own SymTable for runtime variable storage
```

---

## File Map

| File | Purpose |
|------|---------|
| `include/token.h` | `TokenType` enum + `Token` struct + `token_type_to_str()` inline helper |
| `include/lexer.h` | `Lexer` struct declaration |
| `src/lexer.c` | `lexer_init`, `lexer_next` — tokenises a `const char*` source string |
| `include/ast.h` | `NodeType` enum + `ASTNode` union struct + factory function declarations |
| `src/ast.c` | `make_*` constructors, `print_ast`, `free_ast` |
| `include/symtable.h` | `Symbol` + `SymTable` structs |
| `src/symtable.c` | `linear_search`, `symtable_init`, `symtable_set`, `symtable_get`, `symtable_exists` |
| `include/parser.h` | `Parser` struct (Lexer + current Token + SymTable) + parse function declarations |
| `src/parser.c` | `parse_factor`, `parse_term`, `parse_expr`, `parse_statement`, `parse_block`, `parse_program` |
| `include/codegen.h` | `codegen` function declaration |
| `src/codegen.c` | `codegen(ASTNode*, VM*)` — recursive AST walker that emits opcodes |
| `include/vm.h` | `OpCode` enum + `Instruction` struct + `VM` struct |
| `src/vm.c` | `vm_init`, `vm_emit`, `vm_run` — stack-based execution loop |
| `src/main.c` | Entry point: init parser → parse_program → print_ast → codegen → vm_run → free |
| `Makefile` | Single-command build: `make` |

---

## Language Supported (Day 9)

```
let x = 5;
if (x > 3) { print x } else { print 0 }
```

### Statements
- `let <name> = <expr>` — variable declaration and assignment
- `print <expr>` — print a value to stdout (as `%.02f`)
- `if (<cond>) { <stmts> } else { <stmts> }` — conditional (else is optional)

### Expressions
- Arithmetic: `+`, `-`, `*`, `/` (correct precedence via parse_term/parse_expr)
- Comparison: `>`, `<`, `==`
- Parenthesised expressions: `(<expr>)`
- Number literals: `5`, `3.14`
- Variable references: `x`

### Tokens
`TOKEN_NUMBER`, `TOKEN_IDENTIFIER`, `TOKEN_LET`, `TOKEN_IF`, `TOKEN_ELSE`,
`TOKEN_WHILE` *(keyword lexed, not yet parsed)*,
`TOKEN_PRINT`, `TOKEN_PLUS`, `TOKEN_MINUS`, `TOKEN_STAR`, `TOKEN_SLASH`,
`TOKEN_ASSIGN`, `TOKEN_LT`, `TOKEN_GT`, `TOKEN_EQ`,
`TOKEN_LPAREN`, `TOKEN_RPAREN`, `TOKEN_LBRACE`, `TOKEN_RBRACE`,
`TOKEN_SEMICOLON`, `TOKEN_EOF`, `TOKEN_UNKNOWN`

---

## VM Instruction Set

| OpCode | Operand used | Description |
|--------|-------------|-------------|
| `OP_PUSH` | `operand` (double) | Push a literal value onto the stack |
| `OP_ADD` | — | Pop two, push sum |
| `OP_SUB` | — | Pop two, push difference (b − a) |
| `OP_MUL` | — | Pop two, push product |
| `OP_DIV` | — | Pop two, push quotient (errors on divide-by-zero) |
| `OP_GT` | — | Pop two, push 1.0 if a > b else 0.0 |
| `OP_LT` | — | Pop two, push 1.0 if a < b else 0.0 |
| `OP_EQ` | — | Pop two, push 1.0 if a == b else 0.0 |
| `OP_STORE` | `name` | Pop stack top, store into VM symtable |
| `OP_LOAD` | `name` | Push value from VM symtable onto stack |
| `OP_PRINT` | — | Pop stack top, `printf("%.02f\n", val)` |
| `OP_JMP` | `operand` (index) | `i = operand - 1` (unconditional jump) |
| `OP_JMP_IF_FALSE` | `operand` (index) | Pop; if 0.0, jump to index |
| `OP_HALT` | — | Stop execution |

### Codegen pattern for `if/else` (backpatching)
```
[condition code]
OP_JMP_IF_FALSE → ? (patched to start-of-else after emitting then-body)
[then body code]
OP_JMP → ?         (patched to after-else after emitting else-body)
[else body code]
```

---

## Key Design Decisions

### Dual symbol tables
- **`Parser.symtable`** — populated at parse time after each `let`. Only used to validate that a variable was declared before being used in `parse_factor`. Values don't matter (always set to `0`).
- **`VM.symtable`** — populated at runtime by `OP_STORE`. Holds the actual computed values.

### Operator precedence via separate parse functions
```
parse_expr   →  + and −  (lowest precedence)
parse_term   →  * and /
parse_factor →  literals, identifiers, (expr)  (highest)
```

### Comparisons are outside the expression grammar
`>`, `<`, `==` are parsed explicitly inside `parse_statement`'s if-handler, not as part of `parse_expr`. This means they can only appear as the top-level condition of an `if`, not nested inside arithmetic.

### Semicolons are optional
Both `parse_block` and `parse_program` consume `;` if present but don't error if absent.

---

## Bugs Fixed History

| Day | File | Bug | Root Cause |
|-----|------|-----|-----------|
| ≤8 | `lexer.c` | Infinite loop on identifiers | `isalnum(l->pos++)` passed the post-increment return value (always `1`) instead of the char at that position |
| 9 | `ast.c:109` | Infinite loop in `print_ast` for `NODE_IF` | `for(int i = 0; depth + 1; i++)` — missing `i <` made condition always truthy |
| 9 | `codegen.c:42` | `==` operator never emitted `OP_EQ` | Compared `compare.op` against `"="` instead of `"=="` |

---

## Natural Next Steps (Day 10+)

- `while` loops — `TOKEN_WHILE` is already in the lexer/token enum, just needs parser + codegen support
- Variable reassignment (`x = expr` without `let`)
- Nested `if` statements
- Multiple test programs (move hardcoded string out of `main.c`)
- String literals / print with string
- User-defined functions

---

## Build & Run

```bash
make
./compiler
```

Expected output for Day 9 test input:
```
AST: 
LET(x)
  NUMBER(5)

IF
 CONDITION:
    COMPARE(>):
      IDENT(x)
      NUMBER(3)
 THEN: 
    PRINT
      IDENT(x)
 ELSE: 
    PRINT
      NUMBER(0)

OUTPUT: 
5.00
```
