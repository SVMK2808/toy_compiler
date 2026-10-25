---
title: Building a Verification-Aware Programming Language from Scratch in C (with Z3)
published: true
tags: compilers, c, computerscience, programming
cover_image: 
description: How I built Vera, a custom C-based compiler and stack VM featuring compile-time formal verification via Weakest Preconditions and the Z3 SMT solver.
---

Most modern programming languages rely on runtime assertions or unit tests to catch bugs. While useful, runtime checks only catch errors along paths your test suite actually executes. 

What if a programming language could **mathematically prove** that assertions, loop invariants, and function contracts hold across *all* possible inputs before generating a single line of bytecode?

Over the past few weeks, I designed and built **Vera**—a compiled language written entirely from scratch in C, featuring a custom stack-based Virtual Machine and an integrated static verification engine powered by the **Z3 Theorem Prover**.

In this post, I'll walk through how Vera works, how the compiler pipeline is structured, and how we implement **Weakest Precondition (WP) calculus** to verify programs at compile time.

---

## 1. What Does Vera Code Look Like?

Vera looks similar to a C/Rust hybrid, but contracts and invariants are first-class language constructs.

### Function Contracts (`requires` and `ensures`)
Functions can specify preconditions (`requires`) and postconditions (`ensures`), with the special `result` identifier representing the returned value:

```rust
fn add(x, y) requires x > 0 ensures result > y {
    return x + y;
}

print add(5, 10); // Output: 15.00
```

### Loop Invariants
Invariants can be placed on `while`, `for`, and `do-while` loops:

```rust
for (let i = 0; i < 5; i = i + 1) invariant (i >= 0) {
    print i;
}
```

If any contract, invariant, or assertion can fail, the compiler halts at compile-time and outputs a concrete **counterexample**!

---

## 2. Compiler Architecture

Vera follows an end-to-end compiler pipeline:

```
 Source Code (*.ver)
        │
        ▼
   [ Lexer ]          Tokenizes input characters into tokens
        │
        ▼
   [ Parser ]         Recursive-descent parser producing an AST
        │
        ▼
 [ Verification ]     Static verification via Weakest Preconditions (Z3)
        │             (Compilation ABORTS if Z3 finds a counterexample)
        ▼
   [ Codegen ]        Walks the AST and emits stack bytecode instructions
        │
        ▼
     [ VM ]           Executes bytecode on a stack machine with heap/callframes
```

---

## 3. How Compile-Time Static Verification Works

Instead of only verifying contracts at runtime inside the VM, Vera performs **Verification Condition Generation (VCG)** using **Dijkstra's Weakest Precondition (WP) calculus**.

### The Core Idea of Weakest Precondition
For any statement `S` and desired postcondition `Q`, `WP(S, Q)` computes the least restrictive condition that must hold *before* executing `S` to guarantee that `Q` holds *after* `S`.

Here is how Vera computes `WP` across different AST nodes:

* **Assignment / Let (`x = E`):**
  ```text
  WP(x = E, Q) = Q[x := E]
  ```
  *(Syntactic substitution: replace every occurrence of variable `x` in `Q` with expression `E`)*

* **Assertions (`assert C`):**
  ```text
  WP(assert C, Q) = C && Q
  ```

* **Conditionals (`if (C) S1 else S2`):**
  ```text
  WP(if, Q) = (C => WP(S1, Q)) && (!C => WP(S2, Q))
  ```

* **Loops (`while (C) invariant (Inv) { Body }`):**
  A loop generates three independent verification obligations:
  1. **Loop Entry:** The invariant must hold before the loop starts.
  2. **Loop Preservation:** Assuming `Inv && C`, executing the loop body must re-establish `Inv`:
     ```text
     Inv && C => WP(Body, Inv)
     ```
  3. **Loop Exit:** When the loop terminates, the invariant and negated condition must establish the outer postcondition:
     ```text
     Inv && !C => Q
     ```

---

## 4. Bridging C to Z3 via SMT-LIB2

Once Vera constructs the top-level Verification Condition (VC) as an AST formula, it translates it into the **SMT-LIB2** standard format and passes it to the Z3 SMT solver.

### The Sort Mismatch Challenge
In the runtime VM, all variables and numbers are stored as `double` (Real). But in mathematical logic, boolean operators (`and`, `or`, `not`) expect `Bool` sorts.

To prevent Z3 sort mismatch errors, the AST-to-SMT serializer inspects node expressions. If an arithmetic expression is used inside a boolean context, it automatically coerces it to SMT `Bool`:

```c
void print_smt_bool(FILE *out, ASTNode *node) {
    if (is_smt_bool(node)) {
        print_smt_expr(out, node);
    } else {
        // Coerce Real numeric value to Bool: (not (= E 0.0))
        fprintf(out, "(not (= ");
        print_smt_expr(out, node);
        fprintf(out, " 0.0))");
    }
}
```

### Proving Validity by Searching for Counterexamples
To prove that a verification condition `VC` is **always true (valid)**, we ask Z3 if its negation is **satisfiable**:

```text
Assert( not VC )
```

* If Z3 returns **`unsat`**: No input can ever violate the condition. **The program is proven correct!**
* If Z3 returns **`sat`**: Z3 found a concrete assignment of variables that breaks the code. **Verification fails.**

---

## 5. Extracting Concrete Counterexamples

When Z3 returns `sat`, we don't just want to tell the developer "Verification Failed". We want to tell them **which inputs** caused the failure.

We append `(get-model)` to the SMT query and parse Z3's output model in C:

```smt2
(declare-const x Real)
(declare-const y Real)
(assert (not (=> (> x 0.0) (> (+ x y) (+ y 10.0)))))
(check-sat)
(get-model)
```

For this buggy function:

```rust
fn bad_add(x, y) requires x > 0 ensures result > y + 10 {
    return x + y;
}
```

Running the compiler immediately halts with:

```text
Verification FAIL: Function contract for 'bad_add' (Z3 output: sat)
Counterexample values: 
  x = 1.0
  y = 0.0
Error: Static verification failed. VM execution aborted.
```

Z3 found that with `x = 1.0` (which satisfies `x > 0`) and `y = 0.0`, the returned result `1.0` is not greater than `y + 10 = 10.0`, saving you from a runtime defect!

---

## 6. Key Takeaways & Lessons Learned

1. **AST Memory Ownership:** Verification condition generation involves heavy AST substitution and synthesis. Strict cloning (`clone_ast`) is critical to avoid double-free errors during AST destruction.
2. **First-Principles Systems Programming:** Writing a compiler and bytecode VM from scratch in C gives an unmatched appreciation for how call frames, instruction pointers, symbol tables, and heap memory interact.
3. **Formal Methods in Compilers:** Integrating SMT solvers at compile time makes writing reliable software intuitive and mathematically rigorous.

---

## What's Next?

Future directions for **Vera**:
- [ ] Static array bounds checking (`0 <= i < length`).
- [ ] Static division-by-zero detection.
- [ ] A dedicated Web-based IDE with live verification diagnostics and error squiggles.

Check out the full source code and test scripts on GitHub:
👉 **[https://github.com/SVMK2808/toy_compiler](https://github.com/SVMK2808/toy_compiler)**

---

*I’d love to hear your thoughts, suggestions, and feedback in the comments below! Have you experimented with formal verification or building toy compilers? Let's discuss!*
