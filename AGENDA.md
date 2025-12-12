# Scalable, Compiled, Unambiguous Low-Level Language (Scull)

- NEC - Necessary
- EXP - Experimental

## Compiler Related

- [ ] NEC Switch over to LLVM backend (use c bindings)

- [ ] NEC Arenas / Allocators instead of the current allocation method
  - https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory

- [ ] NEC Test cases

- [ ] EXP Bootstrapping Strategy: (inspired from Zig)
  - Generated C code (one file) of the compiler source ships with release (need custom c backend for this)
  - User compiles the bootstrap binary with the system c compiler
  - User uses the bootstrap binary to compile source
  - Native scull binary available

## Language Related

- [ ] NEC make functions and arrays better, fix general bugs and finnickiness

- [ ] NEC Strings

- [ ] NEC io.scl refresh using functions

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
