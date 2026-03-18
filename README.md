# Lubex Compiler & Core Libs

Lubex Compiler is a hobbyist project: a small, educational programming language with its own minimal runtime and core std library. 
It compiles source code into ELF64-x86-64 binaries and provides basic types and operations.

## Requirements

- GCC compiler with C++23 and C23 support
- LLVM IR API v15 (for C++ backend)
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
./build.sh -DBUILD
```

Generated binaries to use will be placed in the `build/` directory.

## Repository Structure

- `compiler/` - Compiler source files
    - `build/` - Generated build files
    - `includes/` - Header files
    - `src/` - Source `.cpp` files
- `runtime/` - Runtime source files
    - `build/` - Generated build files
    - `includes/` - Header files
    - `src/` - Source `.c` files
- `standard/` 
    - `build/` - Generated build files
    - `includes/` - Header files
    - `src/` - Source `.c` files

## Authors

**Iskra** - [GitHub Profile](https://github.com/Iskra-YT)
