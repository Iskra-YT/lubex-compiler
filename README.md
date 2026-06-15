# Lubex Compiler & Core Libs

Lubex is a hobbyist, educational programming language compiler that compiles Lubex source code into **ELF64 x86-64 native binaries**. It features a custom runtime library and standard library, providing basic types, memory management, and I/O operations.

## Features

- **Hand-written lexer, parser, and code generator** (C++23, LLVM 15 backend)
- **Static type system** with type inference, nullable types, and null safety (`?`, `??`, `?.`, `?:`)
- **Object-oriented features**: classes, inheritance (`extends`), method overriding (`override`)
- **Visibility modifiers**: `public`, `private`, `internal`
- **Constant folding** optimizer
- **Custom intermediate representation** (LIR) before LLVM IR generation
- **Freestanding runtime library** (C23, no libc dependency)
- **Standard library** with `Console` and `Math` modules
- **GoogleTest** unit tests and integration tests

## Requirements

- GCC 13+ (with C++23 and C23 support)
- LLVM IR API v15
- CMake >= 3.28
- Ninja build system
- ccache

Install required packages using the provided script: `setup.sh` (for Debian/Ubuntu systems)

## Building

Use the provided build script:

```bash
# Release build
./build.sh

# Debug build with debug prints
./build.sh -DDEBUG
```

Generated binaries will be placed in the `build/` directory.

## Quick Start

```bash
# Initialize a new project
./build/lubex init myProject
cd myProject

# Build the project
../build/lubex build

# Run the generated binary
./build/myProject
```

## Running Tests

```bash
./test.sh
```

This runs all three test suites:
1. Runtime unit tests (C runtime library)
2. Compiler unit tests (lexer, parser, evaluator, LIR, optimizer)
3. Integration tests (end-to-end compilation)

## Repository Structure

| Directory | Description |
|-----------|-------------|
| `compiler/` | Lubex compiler (C++23, LLVM 15) |
| `compiler/src/` | Compiler source files |
| `compiler/includes/` | Compiler headers |
| `compiler/tests/` | GoogleTest unit tests |
| `runtime/` | Runtime library (C23, freestanding) |
| `runtime/src/` | Runtime source files |
| `runtime/includes/` | Runtime headers |
| `runtime/tests/` | Runtime unit tests |
| `standard/` | Standard library (Console, Math) |
| `standard/lubex/` | Lubex module declarations |
| `docs/` | Language documentation |
| `testProject/` | Sample Lubex project |

## Project Components

### Compiler (`compiler/`)

The compiler is written in **C++23** and uses **LLVM 15** as its code generation backend. The compilation pipeline:

1. **Lexer** - Tokenizes source code into tokens
2. **Parser** - Instruction-set based recursive descent parser producing an AST
3. **Optimizer** - Constant folding optimization pass
4. **Evaluator** - Three-pass semantic analysis (declaration, midpass, type-check)
5. **LIR Generator** - Converts AST to Lubex Intermediate Representation
6. **LLVM Emitter** - Generates LLVM IR and emits `.o` object files
7. **Linker** - Links runtime and standard library to produce native ELF64 binary

### Runtime Library (`runtime/`)

Written in **C23** in a freestanding environment (no libc). Provides:
- Heap memory allocator (bitmap-based block allocator)
- Number type (`_BI_Number`) with arithmetic operations
- String type (`_BI_String`)
- Object and Void base types
- OS syscall wrappers (mmap, write, exit)
- RTTI (type information) support

### Standard Library (`standard/`)

Provides user-facing modules:
- `Console` - Output functions (`write`, `writeLn`)
- `Math` - Mathematical functions (`power`)

## Language Documentation

See the [`docs/`](docs/) directory for:
- [Getting Started Guide](docs/getting-started.md)
- [Language Reference](docs/core/)
- [Standard Library Reference](docs/std/)

## Contributing

We welcome contributions!

If you want to help:
- Report issues or suggest new features via [GitHub Issues](https://github.com/Iskra-YT/lubex-compiler/issues).
- Submit pull requests with new modules or improvements.

## Authors

**Iskra** - [GitHub Profile](https://github.com/Iskra-YT)
