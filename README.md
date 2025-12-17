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
* Ability to use Python's `print()` function only with a single integer argument, which gets compiled down to calling `printf("%d\n", ...)` (via a library call to libc, at least on Linux)

## Non-features

* No support for `import`
* No support for floating-point, strings, lists, tuples, sets, or any datatypes other than integers
* No support for lambdas or functions as data/arguments

## Usage syntax

* Input-file as main argument, use `-o` to specify output executable otherwise it's `a.out`, and adding `-c` results in creating an `.o` file, much like gcc

## Implementation

* Project is written in C/C++, uses standard tools like `flex` and `bison` for parsing
* Should work on both x86 and ARM, which suggests we may be best off using an architecture-portable intermediate format such as LLVM-IR

## Sample usages

Create a test program that implements a factorial using recursion
