# Daily Progress Log

---

## Day 13 — User-defined functions, local/global scoping & call frames ✅

**Date:** 2026-06-26

### What was added
- User-defined function syntax: `fn name(param1, param2) { body }` and call syntax: `name(arg1, arg2)`.
- Explicit `return <expr>` statement support.
- New tokens: `TOKEN_FN`, `TOKEN_RETURN`, `TOKEN_COMMA`.
- New AST nodes: `NODE_FUNC_DEF`, `NODE_FUNC_CALL`, `NODE_RETURN`.
- New VM structures: `CallFrame` (for tracking local symbol scopes and return addresses), `FuncDef`, and `FuncTable`.
- New VM opcodes: `OP_CALL`, `OP_RETURN`, `OP_LOAD_LOCAL` (searches locals first, then falls back to globals), `OP_STORE_LOCAL` (declares local variable), and `OP_ASSIGN_VAR` (resolves variable scope dynamically for reassignments).
- Codegen: updated signature to thread an `in_func` context boolean. Uses `OP_ASSIGN_VAR` for all variable reassignments.
- Scoping rules verified:
  1. Reading a global variable from inside a function body.
  2. Overwriting/reassigning a global variable from inside a function body.
  3. Shadowing global variables using local parameters (leaving globals untouched).

### Test input (Scoping & Shadowing)
```c
let g = 10;
fn update() {
    g = 20;
}
fn shadow(g) {
    print g;
}
shadow(5);
print g;
update();
print g;
```

### Output
```
AST: 
LET(g)
  NUMBER(10)

 FUNC_DEF(update) params = []
  ASSIGN(g)
    NUMBER(20)

 FUNC_DEF(shadow) params = [g]
  PRINT
    IDENT(g)

 FUNC_CALL(shadow)
    NUMBER(5)

PRINT
  IDENT(g)

 FUNC_CALL(update)

PRINT
  IDENT(g)

OUTPUT: 
5.00
10.00
20.00
```

---

## Day 12 — for loop + OP_LT bug fix ✅

**Date:** 2026-06-25

### What was added
- `for (init; cond; incr) { body }` statement parsing and codegen
- `TOKEN_FOR` keyword in lexer + token.h
- `NODE_FOR` AST node: init, condition, increment, body, body_count
- Codegen: emit init once, then `loop_start → condition → JMP_IF_FALSE → body → incr → JMP(loop_start)`
- `init` and `incr` parsed via `parse_statement()` — handles both `let` and `x = expr`

### Bug fixed
- `OP_LT` in `vm.c` had `a`/`b` variable names swapped vs `OP_GT` convention — was checking `right < left` instead of `left < right`

### Test input
```
for (let i = 0; i < 5; i = i + 1) { print i }
```

### Output
```
0.00
1.00
2.00
3.00
4.00
```

---

## Day 11 — do-while loop, variable reassignment ✅

**Date:** 2026-06-24

### What was added
- `do { <body> } while (<cond>)` — body executes before condition is checked
- `TOKEN_DO` keyword in lexer + token.h
- `NODE_DO_WHILE` AST node; codegen: body → condition → JMP_IF_FALSE/JMP
- `peek_next()` helper in parser.c — snapshots/restores Lexer state to look ahead without consuming
- Variable reassignment: `x = expr` (without `let`)
- `NODE_ASSIGN` AST node; codegen: evaluate RHS → `OP_STORE`

### Test input
```
let x = 3; while (x > 0) { print x; x = x - 1 }
```

### Output
```
3.00
2.00
1.00
```

---

## Day 10 — while loop support ✅

**Date:** 2026-06-23

### What was added
- `while (<cond>) { <body> }` statement parsing (`parse_statement`)
- `NODE_WHILE` AST node with `condition`, `body[]`, `body_count`
- `make_while()` constructor, `print_ast` + `free_ast` cases
- Codegen with backpatching: `OP_JMP_IF_FALSE` (exit) + `OP_JMP` (loop back)
- Fixed gcc warning: explicit `NODE_NUMBER`/`NODE_IDENT` cases in `free_ast` switch

### Note
While loops are structurally complete but fully useful loops require variable reassignment (`x = expr`) — planned for Day 11.

### Test input
```
let x = 0; while (x > 1) { print x }
```

### Output
```
OUTPUT:
(empty — condition false on entry, loop body never executes)
```

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
