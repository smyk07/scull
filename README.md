# Scull

A C and Go like systems level programming language. Final name yet to be decided.
Only linux systems are supported for now.

```
-include "io.scl"

fn main() : int {
  printf("Hello, World\n")
}
```

## Inspiration

- [The C Programming Language](<https://en.wikipedia.org/wiki/C_(programming_language)>)
- [The Go Programming Language](https://go.dev)
- [How to build a compiler from scratch by Alex The Dev](https://youtu.be/HOe2YFnzO2I)

## Prerequisites for building

- [`llvm`](https://llvm.org/)
- [`make`](https://www.gnu.org/software/make)
- [`clang`](https://clang.llvm.org)
- [`bear`](https://github.com/rizsotto/Bear) (for `compile_commands.json`)

## Tools required

- [`clang`](https://clang.llvm.org) (for linking)

## Usage

Build and install the compiler:

```
make install
```

Compile the examples in `./examples` like so:

```
sclc -i ./lib ./examples/n_prime_numbers.scl
```

Run the executable:

```
./examples/n_prime_numbers
```
