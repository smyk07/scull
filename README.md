# Scull

<img src="logo/logo.jpg" alt="SCULL Logo" height="150" title="Thanks, Vismita!">

A C and Go like systems level programming language. Final name yet to be decided.
Only linux systems are supported for now.

```
-include "io.scl"

fn main() : int {
  printf("Hello, World!\n")
  return 0
}
```

Compile and run:

```
# clone and cd to the repo
$ git clone https://github.com/smyk07/scull.git
$ cd scull

# sync submodules and build llvm
$ make llvm-sync llvm

# build sclc
$ make

# compile the program
$ ./bin/sclc -i ./lib hello.scl

# run the compiled binary
$ ./hello
```

More examples in `./examples`.

## Inspiration

- [The C Programming Language](<https://en.wikipedia.org/wiki/C_(programming_language)>)
- [The Go Programming Language](https://go.dev)
- [How to build a compiler from scratch by Alex The Dev](https://youtu.be/HOe2YFnzO2I)

## Prerequisites for building

- [`make`](https://www.gnu.org/software/make)
- [`clang`](https://clang.llvm.org)
- [`bear`](https://github.com/rizsotto/Bear) (for `compile_commands.json`)

## Tools required

- [`lld`](https://lld.llvm.org/) (for linking)
