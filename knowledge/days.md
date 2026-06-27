# Daily Progress Log

---

## Day 14 — Comparisons in expressions, logical operators & booleans ✅

**Date:** 2026-06-27

### What was added
- Integrated comparison operators (`>`, `<`, `==`, `<=`, `>=`) directly into expressions rather than statement-level syntax only.
- Added logical AND (`&&`), logical OR (`||`), and logical NOT (`!`) operators with standard C operator precedence.
- Added boolean literals `true` (evaluates to `1.0`) and `false` (evaluates to `0.0`).
- Restructured expression grammar hierarchy in the parser: `parse_expr` -> `parse_logical_and` -> `parse_equality` -> `parse_comparison` -> `parse_add_sub` -> `parse_term` -> `parse_unary` -> `parse_factor`.
- Simplified condition parsing in `if`, `while`, `do-while`, and `for` loops to a single `parse_expr(p)` call.
- Implemented short-circuit logic for `&&` and `||` in codegen using conditional jumps (`OP_JMP_IF_FALSE`, `OP_JMP`).
- Added VM opcodes: `OP_NOT` (logical negation), `OP_LE` (`<=`), and `OP_GE` (`>=`).
- Compiled unary arithmetic `-expr` to `0.0 - expr`.

### Test input
```c
let x = 5;
let y = 10;
let cond = (x < y) && (y >= 10) && !false;
if (cond) {
    print 1;
} else {
    print 0;
}
print -x;
print (x > 10) || false;
```

### Output
```
OUTPUT:
1.00
-5.00
0.00
```

---

## Day 13 — User-defined functions, local/global scoping & call frames ✅

**Date:** 2026-06-26

### What was added
- User-defined function syntax: `fn name(param1, param2) { body }` and call syntax: `name(arg1, arg2)`.
- Explicit `return <expr>` statement support.
- New tokens: `TOKEN_FN`, `TOKEN_RETURN`, `TOKEN_COMMA`.
- New AST nodes: `NODE_FUNC_DEF`, `NODE_FUNC_CALL`, `NODE_RETURN`.
- New VM structures: `CallFrame` (for tracking local symbol scopes and return addresses), `FuncDef`, and `FuncTable`.
- New VM opcodes: `OP_CALL` (resolves function start, pushes CallFrame, binds parameters), `OP_RETURN` (pops CallFrame, pushes return value, jumps back), `OP_LOAD_LOCAL` (searches locals first, falls back to globals), `OP_STORE_LOCAL` (stores to local call frame), and `OP_ASSIGN_VAR` (resolves scope dynamically for variable reassignments).
- Codegen: updated signature to thread an `in_func` context boolean. Uses `OP_ASSIGN_VAR` for all variable reassignments.
- Scoping rules verified:
  1. Reading a global variable from inside a function body.
  2. Overwriting/reassigning a global variable from inside a function body.
  3. Shadowing global variables using local parameters (leaving globals untouched).

### Test input (Recursive Factorial & Scoping/Shadowing)
```c
// 1. Recursion test
fn fact(n) {
    if (n < 2) {
        return 1;
    }
    return n * fact(n - 1);
}
let result = fact(5);
print result;

// 2. Scoping and Shadowing test
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
// Recursion output
120.00

// Scoping & Shadowing output
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
