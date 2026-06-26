# Bug Log

A running log of all bugs found and fixed across the daily compiler build sessions.

---

## Day 13 — VM local stack initialization & compilation typos

### 1. VM stack initialization
- **File:** `src/vm.c`
- **Symptom:** Running the compiled compiler binary produced a `zsh: bus error ./compiler`.
- **Root cause:** `vm_init` was not updated to initialize the new fields `frame_top` and `func_table.count`. Since it was initialized with garbage memory on the stack, `OP_CALL` was trying to write to `vm->frames[vm->frame_top]` where `frame_top` was a random massive integer, triggering a segmentation fault/bus error.
- **Fix:** Added `vm->frame_top = -1;` and `vm->func_table.count = 0;` to `vm_init`.

### 2. Lexer keyword vs Identifier check in parser
- **File:** `src/parser.c`
- **Symptom:** Semicolon and expression parsing behaved incorrectly inside and outside functions.
- **Root cause:** In `parse_statement`, the return check was testing `p -> current.type == TOKEN_IDENTIFIER` instead of `TOKEN_RETURN`, which parsed every variable/identifier statement as a return statement.
- **Fix:** Corrected the check to `p -> current.type == TOKEN_RETURN`.

### 3. codegen.c NODE_LET local opcode typo
- **File:** `src/codegen.c`
- **Symptom:** Variable declarations (`let`) inside functions did not work correctly or failed to store variables in the local call frame.
- **Root cause:** The ternary expression in `NODE_LET` emitted `OP_LOAD_LOCAL` instead of `OP_STORE_LOCAL` when `in_func` was true: `vm_emit(vm, in_func ? OP_LOAD_LOCAL: OP_STORE, 0, node -> let.name);`.
- **Fix:** Changed to `OP_STORE_LOCAL`.

### 4. ast.c print_ast and free_ast recursive traversal bugs
- **File:** `src/ast.c`
- **Symptom:** Printing the AST for functions iterated over parameter count instead of body count. Deallocating function memory caused leaks/invalid frees.
- **Root cause:** 
  1. `print_ast` for `NODE_FUNC_DEF` printed the body using `i < node -> func_def.param_count` instead of `body_count`.
  2. `free_ast` for `NODE_RETURN` used `free(node -> ret_val)` directly instead of recursively calling `free_ast(node -> ret_val)`.
- **Fix:** Fixed the loop condition in `print_ast` to use `body_count`, and updated `free_ast` to recursively free the return value ASTNode.

---

## Day 12 — OP_LT operand order reversed

**File:** `src/vm.c`  
**Symptom:** `for` loop with `<` condition produced no output (condition evaluated false on first check even when true).  
**Root cause:** `OP_LT` popped `a` (top of stack = right operand = 5.0) then `b` (left operand = 0.0) and checked `a < b` (i.e. `5.0 < 0.0`). Inconsistent with `OP_GT` which correctly pops `b` first (right) then `a` (left) and checks `a > b`.  
**Fix:** Renamed the two pop variables in `OP_LT` to match `OP_GT` convention: `b = pop()` (right), `a = pop()` (left), check `a < b`.

---

## Day ≤ 8 — Lexer infinite loop on identifiers

**File:** `src/lexer.c`  
**Symptom:** Program hung immediately, printing nothing, when any identifier was lexed.  
**Root cause:**
```c
// ❌ Before fix
while(isalnum(l->src[l->pos++]) || ...) {
    buf[len++] = ...
}
```
`l->pos++` returns the old value of `pos` (an integer index, typically small like `0`, `1`, etc.). `isalnum(0)` and `isalnum(1)` are both false (ASCII 0 = NUL, ASCII 1 = SOH — not alphanumeric), so the loop body never executed, `pos` never advanced, and the outer `lexer_next` call looped forever.

**Fix:**
```c
// ✅ After fix
while(isalnum(l->src[l->pos]) || l->src[l->pos] == '_') {
    buf[len++] = l->src[l->pos++];
}
```

---

## Day 9 — `print_ast` infinite loop on `NODE_IF`

**File:** `src/ast.c`, line 109 (now fixed)  
**Symptom:** Program hung after printing the `COMPARE` node of the IF condition. The `THEN:` label was never printed.  
**Root cause:**
```c
// ❌ Before fix
for(int i = 0; depth + 1; i++) printf(" ");
```
`depth + 1` evaluates to a non-zero integer — always truthy in C. The loop spun forever printing spaces.

**Fix:**
```c
// ✅ After fix
for(int i = 0; i < depth + 1; i++) printf(" ");
```

---

## Day 9 — `==` operator never emitted `OP_EQ`

**File:** `src/codegen.c`, line 42 (now fixed)  
**Symptom:** Equality comparisons silently failed (fell through without emitting any opcode).  
**Root cause:**
```c
// ❌ Before fix — compares against "=" (single equals)
else if(strcmp(node->compare.op, "=") == 0) vm_emit(vm, OP_EQ, 0, NULL);
```
The parser stores the comparison operator as `"=="` (via `strncpy(op, "==", 3)`), so the `strcmp` never matched.

**Fix:**
```c
// ✅ After fix
else if(strcmp(node->compare.op, "==") == 0) vm_emit(vm, OP_EQ, 0, NULL);
```
