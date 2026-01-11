# Scalable, Compiled, Unambiguous Low-Level Language (Scull)

- NEC - Necessary
- EXP - Experimental

## Compiler Related

- [x] NEC Seperate backend from frontend, have a clean backend interface integrate with main driver `src/sclc.c`

- [x] NEC Switch over to LLVM backend (use c bindings)
  - Targets: x86_64, ARM64, RISCV64

- [x] NEC Handle LLVM target selection in cstate, rather than in backend.c or llvm.c

- [x] NEC -S cli flag and --emit-llvm flag, and general CLI improvements

- [x] NEC Parser bug fixes and proper memory management

- [x] NEC Better error handling

- [ ] NEC Test cases
  - Lexer
  - Parser
  - Semantic
  - Codegen / Backend
  - End-to-End

- [ ] NEC Arenas / Allocators (for the full compiler) instead of the current allocation method
  - [ ] Better arena implementation with chaining

- [x] NEC Seperate stack based and heap based allocations, see whats really needed and whats unnecessary in sclc

- [ ] NEC only emit the output file in current directory unless specified, else use `/tmp`

- [ ] NEC Develop C Backend
  - Language related changes which would facilitate 1:1 C translation are needed (basic types, structs, unions, enums, memory management, etc)

- [ ] EXP Bootstrapping Strategy: (inspired from Zig)
  - Generated C code (one file) of the compiler source ships with release (need custom c backend for this)
  - User compiles the bootstrap binary with the system c compiler
  - User uses the bootstrap binary to compile source
  - Native scull binary available

## Language Related

- [x] NEC Strings

- [ ] NEC Negative numbers and u32 and i32 - like types

- [ ] NEC For loop

- [ ] NEC proper control flow (else, switch)

- [ ] NEC Structs, Unions, Enums

- [ ] NEC Some alternative for sizeof()

- [ ] NEC Operators: Logical, Bitwise, Assignment, Ternary

- [ ] NEC Macros

- [ ] NEC io.scl refresh using functions

- [ ] NEC Either Static variables and functions, or implementation blocks (pref)

- [ ] NEC Multi-dimensional arrays

- [ ] NEC Start work on the standard library

- [ ] EXP Modular / Constraints in type system (runtime overhead)

```
  int mod(5) counter = 4
```

- [ ] EXP Nullable types

```
  int definately = 9
  int? maybe = null

  int? *definately_ptr = &definately
  int? *maybe_ptr = null
```

- [ ] EXP Arbitrary width for types

```
  int:7 smol = 2
```

- [ ] EXP Result enum similar to rust

```
  fn divide(int a, int b) : Result<int, Err> {
    if (b == 0) then return Err.DivisionByZero
    return a / b
  }
```

- [ ] EXP Distinct typedefs, cannot be cast to other typedefs wrapping over the same types

- [ ] EXP Expansion of above, packed structs (without padding) (also has runtime overhead)

- [ ] EXP Compile time functions (evaluated at compile time)

- [ ] EXP Inline functions (copy and paste like C)
