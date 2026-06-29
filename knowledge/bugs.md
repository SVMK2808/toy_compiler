# Bug Log

A running log of all bugs found and fixed across the daily compiler build sessions.

---

## Day 16 — Function calls blocked & VM Heap overwrites

### 1. Function calls blocked by variable existence check
- **File:** `src/parser.c`
- **Symptom:** Evaluating a function call in the REPL (e.g. `double(4)`) failed with `Error: Undefined variable: (double)`.
- **Root cause:** In the refactored identifier block of `parse_factor()`, the `symtable_exists` verification check was run unconditionally on all parsed identifiers. Because user-defined function names are stored in `VM.func_table` rather than `p->symtable`, they failed the check and triggered the error.
- **Fix:** Added a conditional check `next.type != TOKEN_LPAREN` to only run `symtable_exists` if the identifier is not being used as a function call name.

### 2. VM Heap allocation index overwrite
- **File:** `src/vm.c`
- **Symptom:** Allocating multiple arrays caused them to overwrite each other on the heap, making their pointers collide.
- **Root cause:** In `OP_NEW_ARRAY` handler, we assigned `vm->heap_top = count;` instead of `vm->heap_top += count;`. This reset the heap pointer to the current array's size rather than advancing it contiguously.
- **Fix:** Corrected it to `vm->heap_top += count;`.

---

## Day 15 — REPL duplicate statement executions due to halt early return

### 1. REPL duplicate statement executions due to halt early return
- **File:** `src/vm.c`
- **Symptom:** In the REPL, typing any code statement repeatedly printed the output of all previous statements (e.g. typing `let b = 20;` reprinted the result of `print a + b;`).
- **Root cause:** `vm_run()` contained an early return for `OP_HALT` (`return;`), meaning that the update logic `vm->start_ip = vm->code_count;` placed at the very bottom of `vm_run` was never executed. As a result, `start_ip` remained stale (always starting back at 0 or the previous line), forcing the VM to re-run old bytecode.
- **Fix:** capturing `line_start_ip` inside `main.c`'s compiler loop and explicitly setting `vm->start_ip = line_start_ip` before calling `vm_run()`, then removing the stale update logic from `vm.c` to prevent any index skipping.

---

## Day 14 — Grammar chain integration & Logical codegen jumps

### 1. parse_term bypassing unary parsing
- **File:** `src/parser.c`
- **Symptom:** Compiling and running the compiler produced `Error: unexpected token:` pointing to `-` (in `-x`) or `!` (in `!false`).
- **Root cause:** `parse_term` was still recursively calling `parse_factor(p)` directly, completely skipping the newly introduced `parse_unary(p)` parsing layer. The parser was unable to associate unary operators with factors.
- **Fix:** Changed all recursive lookup calls in `parse_term` to call `parse_unary(p)` instead of `parse_factor(p)`.

### 2. Typo in parser.c function naming
- **File:** `src/parser.c`
- **Symptom:** Linker error `Undefined symbols for architecture arm64: _parse_logical_and`.
- **Root cause:** The implementation of the parser grammar function was named `make_logical_and` instead of `parse_logical_and`.
- **Fix:** Renamed the function to `parse_logical_and`.

### 3. Missing push and label updates in logical OR (||) codegen
- **File:** `src/codegen.c`
- **Symptom:** Logical OR expressions evaluated incorrectly or caused jump errors.
- **Root cause:**
  - The `NODE_LOGICAL` case for `||` was missing the `OP_PUSH 0.0` instruction in the false path.
  - The jumps for successful paths (`jmp_end1` and `jmp_end2`) were not being patched to point past the false path's `OP_PUSH 0.0`.
- **Fix:** Emitted `OP_PUSH 0.0` in the false path and patched both `jmp_end1` and `jmp_end2` to point to the instruction index immediately following the `0.0` push.

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
