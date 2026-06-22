# Bug Log

A running log of all bugs found and fixed across the daily compiler build sessions.

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
