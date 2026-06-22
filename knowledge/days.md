# Daily Progress Log

---

## Day 9 — if/else, comparison operators, full pipeline ✅

**Date:** 2026-06-22  
**Reference:** https://claude.ai/share/e5f6ca38-b7b8-48f0-896b-5b83b78caf6d

### What was added
- `if (<cond>) { <then> } else { <else> }` statement parsing (`parse_statement` + `parse_block`)
- Comparison operators `>`, `<`, `==` (parser + AST `NODE_COMPARE` + codegen `OP_GT/LT/EQ`)
- `NODE_IF` AST node with `condition`, `then_body[]`, `else_body[]`
- `OP_JMP` and `OP_JMP_IF_FALSE` opcodes with backpatching in codegen
- `print_ast` support for `NODE_IF` and `NODE_COMPARE`
- `free_ast` support for `NODE_IF`

### Bugs fixed
- Lexer infinite loop on identifiers (see `bugs.md`)
- `print_ast` infinite loop on `NODE_IF` — `depth + 1` used as loop condition instead of `i < depth + 1`
- `codegen.c` equality op: `"="` → `"=="` in strcmp

### Test input
```
let x = 5; if (x > 3) { print x } else { print 0 }
```

### Output
```
5.00
```

---

## Days 1–8 — (to be backfilled)

*Earlier days built the lexer, token types, AST foundation, arithmetic expressions,*
*variable declaration with `let`, the `print` statement, the symbol table,*
*the stack-based VM, and the codegen for arithmetic and variable ops.*
