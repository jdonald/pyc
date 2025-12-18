# pyc

A rudimentary Python-to-native-binary compiler in C++

## Features

* Types limited to integer-only, 32-bit signed
* `+`, `-`, `*`, `/`, `%` i.e. plus, minus, multiply, divide, remainder arithmetic options
* Ability to assign integer variables with `=`
* Equality and inequality comparisons `==`, `>`, `<`, `>=`, `<=`, which all return integer types (0 for false, 1 for true)
* Basic logic operators `and` and `or`, which lazy-evaluate their arguments
* Order-of-operations for the above operators matches what one would find in C
* Allow parentheses
* Can define custom functions via Python's `def`, that take anywhere from zero to four args, these get compiled to C-style functions in the resulting binary
* In fact to make an executable program we require that there be a `def main()`
* The only code allowed outside a `def` is of the form `if __name__ == '__main__': main()` (because we don't support an interpreter mode)
* Ability to use Python's `print()` function only with a single integer argument, which gets compiled down to calling `printf("%d\n", ...)` (via a library call to libc, at least on Linux)
## Non-features

* No support for `import`
* No support for floating-point, strings, lists, tuples, sets, or any datatypes other than integers
* No support for lambdas or functions as data/arguments
* No global variables (again most code must be within `def` definitions)

## Usage syntax

* Input-file as main argument, use `-o` to specify output executable otherwise it's `a.out`, and adding `-c` results in creating an `.o` file, much like gcc

## Implementation

* Project is written in C/C++, uses standard tools like `flex` and `bison` for parsing
* Should work on both x86 and ARM, which suggests we may be best off using an architecture-portable intermediate format such as LLVM-IR

## Dependencies

* `g++` - C++ compiler with C++14 support
* `flex` - Fast lexical analyzer generator
* `bison` - GNU parser generator
* `llc` - LLVM static compiler (part of LLVM toolchain)
* `gcc` - For linking the final executable

On Ubuntu/Debian:
```bash
sudo apt-get install g++ flex bison llvm gcc
```

## Building the Compiler

```bash
make
```

This will produce the `pyc` executable compiler.

To clean build artifacts:
```bash
make clean
```

## Sample Usage

### Factorial Example

Create a file `factorial.py`:

```python
def factorial(n):
    if n <= 1:
        return 1
    else:
        return n * factorial(n - 1)

def main():
    result = factorial(5)
    print(result)
    return 0

if __name__ == "__main__":
    main()
```

Compile and run:
```bash
./pyc factorial.py -o factorial
./factorial
```

Expected output:
```
120
```

### Simple Example

A minimal program:

```python
def main():
    return 0
```

Compile:
```bash
./pyc minimal.py -o minimal
```

### Creating Object Files

To create an object file instead of an executable:
```bash
./pyc factorial.py -c -o factorial.o
```
