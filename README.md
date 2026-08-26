# Toy Compiler

A stack-based VM and custom compiler built in C. It supports an end-to-end compilation pipeline:
**Lexer ➔ Parser (AST) ➔ Codegen ➔ Stack-based Virtual Machine**.

The compiler includes standard programming language constructs alongside verification-oriented features under **VAPL** (Verification Assisted Programming Language) such as runtime assertions and loop invariants.

---

## Architecture Overview

```
Source Code (*.toy)
       │
       ▼
   [Lexer]          (src/lexer.c, include/lexer.h)
  Tokenizes input into a stream of Token structures
       │
       ▼
  [Parser]          (src/parser.c, include/parser.h)
  Recursive-descent parser. Emits an Abstract Syntax Tree (AST).
  Uses a parse-time symbol table to validate variable declarations.
       │
       ▼
 [Codegen]          (src/codegen.c, include/codegen.h)
  Recursively walks the AST and generates VM bytecode instructions
       │
       ▼
   [VM]             (src/vm.c, include/vm.h)
  Stack-based VM executing instructions with a runtime heap and Call Stack
```

---

## Language Features

### Core Syntax & Types

- **Variables**: Declaration via `let x = <expr>;` and subsequent reassignment via `x = <expr>;`.
- **Numbers**: Double-precision floating-point numbers.
- **Booleans**: `true` (evaluates to `1.0`) and `false` (evaluates to `0.0`).
- **Standard Printing**: `print <expr>` outputs to stdout (formatted to two decimal places).

### Operators

- **Arithmetic**: `+`, `-`, `*`, `/`, and unary `-`.
- **Comparisons**: `>`, `<`, `==`, `<=`, `>=`.
- **Logical**: `&&` (logical AND), `||` (logical OR), `!` (logical NOT). Fully supports short-circuiting.

### Control Flow

- **Conditional**: `if (<cond>) { ... } else { ... }` (else is optional).
- **Loops**:
  - `while (<cond>) { ... }`
  - `do { ... } while (<cond>);`
  - `for (let i = 0; i < n; i = i + 1) { ... }`

### Advanced Features

- **Functions**: User-defined functions with parameter scoping, call frames, and explicit return support:
  ```c
  fn double(x) {
      return x * 2;
  }
  print double(5); // Output: 10.00
  ```
- **Arrays**: Core array storage on a VM sequential heap:
  ```c
  let arr = [10, 20, 30];
  print arr[0]; // 10.00
  arr[1] = 99;
  print arr[1]; // 99.00
  ```
- **Verification Assertions (VAPL)**:
  - **`assert <expr>;`**: Halts the VM execution with an exit status of `1` if the condition is falsy (`0.0`).
  - **`invariant (<expr>)`**: Loop invariants verified at runtime. Placed on `while` loops, checking before loop entry and at the end of each iteration:
    ```c
    let x = 0;
    while (x < 3) invariant (x >= 0) {
        x = x + 1;
    }
    ```

---

## Getting Started

### Prerequisites

Make sure you have `gcc` and `make` installed on your machine.

### Building the Compiler

Run the following command in the workspace directory to build the `./compiler` executable:

```bash
make
```

To remove build artifacts:

```bash
make clean
```

---

## Execution Modes

The compiler supports three modes of execution depending on the inputs:

### 1. REPL Mode (Interactive Shell)

Launch the interactive Read-Eval-Print Loop:

```bash
./compiler
```

This launches a persistent shell where you can type code and execute it line by line:

```
Toy Compiler REPL (Day 15)
Type code and press Enter. Type 'exit' to quit.

toy> let a = 10;
toy> print a * 2;
20.00
toy> exit
```

### 2. File Mode

Pass a source file path to compile and run it. This mode outputs the parsed Abstract Syntax Tree (AST) structure and the final program outputs:

```bash
./compiler test_scripts/01_simple.toy
```

**Example Output**:

```
AST: 
LET(a)
  NUMBER(10)

LET(b)
  NUMBER(20)

LET(c)
  BINOP(+)
    BINOP(*)
      IDENT(a)
      NUMBER(2)
    BINOP(/)
      IDENT(b)
      NUMBER(4)

PRINT
  IDENT(c)

OUTPUT: 
25.00
```

### 3. Piped STDIN Mode

Stream commands or pipe files directly into the compiler. Useful for scripts and shell pipelines:

```bash
echo "let x = 5; print x;" | ./compiler
```

Or:

```bash
cat test_scripts/01_simple.toy | ./compiler
```

---

## Codebase Structure

- [Makefile](file:///Users/svmk/toy_compiler/Makefile): Compiles all source files.
- [include/](file:///Users/svmk/toy_compiler/include): Header files defining data structures.
  - [token.h](file:///Users/svmk/toy_compiler/include/token.h): Token types and conversions.
  - [lexer.h](file:///Users/svmk/toy_compiler/include/lexer.h): Lexical analysis state.
  - [ast.h](file:///Users/svmk/toy_compiler/include/ast.h): AST Node structures and builders.
  - [parser.h](file:///Users/svmk/toy_compiler/include/parser.h): Parser state and token peeking helpers.
  - [symtable.h](file:///Users/svmk/toy_compiler/include/symtable.h): Symbol storage (scopes and identifiers).
  - [vm.h](file:///Users/svmk/toy_compiler/include/vm.h): Opcodes, Instructions, CallFrames, Heap, and VM context.
  - [codegen.h](file:///Users/svmk/toy_compiler/include/codegen.h): Code generator signatures.
- [src/](file:///Users/svmk/toy_compiler/src): Source code implementation files.
  - [main.c](file:///Users/svmk/toy_compiler/src/main.c): Compiler CLI, file-reading, and REPL loop.
  - [lexer.c](file:///Users/svmk/toy_compiler/src/lexer.c): Scanning source characters into tokens.
  - [parser.c](file:///Users/svmk/toy_compiler/src/parser.c): Parsing grammar, expression hierarchies, and control structures.
  - [ast.c](file:///Users/svmk/toy_compiler/src/ast.c): AST creation, memory management, and visual printer.
  - [codegen.c](file:///Users/svmk/toy_compiler/src/codegen.c): Translating AST nodes into virtual machine instructions.
  - [vm.c](file:///Users/svmk/toy_compiler/src/vm.c): Stack machine execution loop, heap storage, frame handling.
  - [symtable.c](file:///Users/svmk/toy_compiler/src/symtable.c): Variable lookup and symbol registration.
- [test_scripts/](file:///Users/svmk/toy_compiler/test_scripts): Directory containing example `.toy` scripts validating various compiler capabilities.
