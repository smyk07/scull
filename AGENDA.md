# Scalable, Compiled, Unambiguous Low-Level Language (Scull)

Please Follow [this](https://www.conventionalcommits.org/en/v1.0.0/) as a commit-message writing guide.

**Legend:**

- NEC - Necessary
- EXP - Experimental
- ? - Unsure and needs research

---

## Phase 1: Frontend and Core Compiler Infrastructure

### Build System, Tooling, Infrastructure & UX

- [ ] NEC Better arena implementation with chaining
- [x] NEC Only emit output file in current directory unless specified with `-c`, else use `/tmp`
- [x] NEC Add `-O` cli option
- [x] NEC Separate backend from frontend, clean backend interface in `src/sclc.c`
- [x] NEC Separate stack-based and heap-based allocations
- [ ] NEX Add option to parse and use -o before/after -c for specifying where the object file should be stored

- [x] NEC make typedefs for ints and floats: u32, u64, f32, etc in utils and use uniformly across codebase

### Error Handling & Debugging

- [ ] NEC Better errors
  - [ ] Fix existing bugs
  - [ ] Errors with source preview
- [x] NEC Tab counting in AST printer
- [x] NEC Parser bug fixes and proper memory management
- [ ] NEC Parser refactor
  - reduce pre-defined parsables into instructions, rather than the existing
  - will follow a similar top-down approach but would be better to add features to and read

### Testing

- [ ] NEC Test cases
  - [ ] Lexer
  - [ ] Parser
  - [ ] Semantic analysis
  - [ ] Codegen / Backend
  - [ ] End-to-End

---

## Phase 2: Backend & Code Generation

### LLVM Backend

- [x] NEC Switch to LLVM backend (C bindings)
  - Targets: x86_64, ARM64, RISCV64
- [x] NEC Handle LLVM target selection in cstate
- [x] NEC `-S` CLI flag and `--emit-llvm` flag, general CLI improvements
- [x] NEC Move linking to llvm backend, since linking is only needed there (For now)

- [ ] NEC Move to llvm submodule instead of system installation of llvm-21
  - [x] basic setup
  - [x] handle linking through cc which is available everywhere
  - [ ] find and fix bugs (eg. `./examples/linear_search.scl`)
        idk this automatically fixed itself somehow

### Alternative Backends

- [ ] NEC Develop C Backend
  - Language changes for 1:1 C translation needed
  - Basic types, structs, unions, enums, memory management, etc.

- [ ] EXP Optional cosmopolitan backend for generating fat binaries

### Distribution

- [ ] EXP Build with [jart/cosmopolitan](https://github.com/jart/cosmopolitan) for fat binary portability
- [ ] EXP Bootstrapping Strategy (inspired by Zig):
  - Generated C code (one file) of compiler source ships with release
  - User compiles bootstrap binary with system C compiler
  - User uses bootstrap binary to compile source
  - Native scull binary available

---

## Phase 3: Core Language Features

### Syntax

- [ ] NEC Write a EBNF Grammar specification for scull

### Type System

- [ ] NEC `void` type
- [ ] NEC Unsigned ints (`u32`) and signed ints (`i32`)
- [ ] NEC Boolean type
- [ ] NEC Typedefs
- [ ] NEC Better way to declare variables: `let x: u32 = 3`
- [ ] ? Type casting
- [ ] ? Type inference

### Control Flow

- [x] NEC If statement
- [x] NEC Else statement
- [x] NEC Else if statement
- [x] NEC Switch/match statement

- [ ] NEC For loop
  - [x] Basic implementation
  - [ ] Typing for iterator
  - [ ] looping through arrays
- [ ] EXP For-each loop

- [ ] NEC Ternary operator-like functionality using if-elseif-else in assignment

- [ ] NEX Seperate scopes for multi-instr conditional blocks (if-else, match, ...)

### Operators

- [ ] NEC Logical operators (`&&`, `||`, `!`)
- [ ] NEC Bitwise operators (`&`, `|`, `^`, `~`, `<<`, `>>`)
- [ ] NEC Assignment operators (`+=`, `-=`, `*=`, `/=`, etc.)

### Basic Features

- [x] NEC Strings
- [x] NEC Negative numbers
- [ ] NEC Multi-dimensional arrays
- [ ] NEC Alternative for `sizeof()`

---

## Phase 4: Advanced Language Features

### User-Defined Types

- [ ] NEC Structs
- [ ] NEC Unions
- [ ] NEC Enums
- [ ] NEC Implementation blocks for user-defined data types
- [ ] EXP Traits / Interfaces

### Functions

- [ ] EXP Anonymous functions

### Metaprogramming

- [ ] NEC Preprocessor macros
- [ ] EXP Compile-time functions (evaluated at compile time)
- [ ] EXP Inline functions (copy and paste like C)

### Standard Library

- [ ] NEC Refresh `io.scl` using functions
- [ ] NEC Standard Library
  - [ ] Dynamic memory management
  - [ ] String manipulation
  - [ ] File I/O
  - [ ] Collections

---

## Phase 5: Experimental Features

### Advanced Type System

- [ ] EXP Nullable types

```scl
  int definately = 9
  int? maybe = null

  int? *definately_ptr = &definately
  int? *maybe_ptr = null
```

- [ ] EXP Optional chaining

```scl
  printf("%s\n", object?.name)
  // Prints null if object.name doesn't exist
```

- [ ] EXP Modular/Constraints in type system (runtime overhead)

```scl
  int mod(5) counter = 4
```

- [ ] EXP Arbitrary width for types

```scl
  int:7 smol = 2
```

- [ ] EXP Distinct typedefs (cannot be cast to other typedefs wrapping the same types)

- [ ] EXP Packed structs without padding (runtime overhead)

- [ ] EXP Sum types (like rust enums)

### Error Handling

- [ ] EXP Result enum similar to Rust

```scl
  fn divide(int a, int b) : Result<int, Err> {
    if b == 0 then return Err.DivisionByZero
    return a / b
  }
```

---

## Abstract Roadmap

### v0.0 (MVP / Diploma Final Year Project Ready)

- Basic compiler infrastructure
- Core type system (int, char, bool, void)
- All control flow
- All operators
- Arrays
- Functions

### v0.1

- C backend + Initiate bootstrap
- Structs, unions, enums
- Implementation blocks
- Standard library basics (dependency on libc can still exist)

### v0.2

- Partial Bootstrap
- Full standard library
- Traits/interfaces

### v1.0+

- Full bootstrap
- Based on user feedback
- Choose 2-3 experimental features to stabilize
