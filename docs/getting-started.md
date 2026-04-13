# Getting Started

This guide will help you install and run your first program using Lubex.

---

## Requirements

Make sure you have the following installed:
- GCC (with C++23 and C23 support)
- LLVM IR API v15
- CMake ≥ 3.28
- Ninja
- ccache

## Setup

Clone the repository:
```bash
git clone https://github.com/Iskra-YT/lubex-compiler.git
cd lubex
```

Install dependencies (Debian/Ubuntu):
```bash
./setup.sh
```

## Build

Build the project using:
```bash
# Release build
./build.sh

# Debug build
./build.sh -DDEBUG
```

After building, the compiler will be available in: `build/`

## Your First Program

Create a project via command:
```bash
./build/lubex init helloProgram
```

After running this command there should append folder `helloProgram` with catalog structure:
- `src/`
- `project.json`


> [!WARNING]
> Presented code will only apear on lubex-r202604a01 and newer versions

At `src/` folder there is a file named `main.lbx`, that contains following code:
```lubex
module main;

import std;

class HelloProgram -> {
    public static func entry(): Number -> {
        std.Console.out("Hello, World!\n");
        return 0;
    };
};
```

## Compile

Run the compiler:
```bash
../build/lubex build
```

This will generate a native executables in `build/`

## Run

Execute the program:
```bash
./build/helloProgram
```

You should see:
```
Hello, World!
```

## What's Next?

- Learn the language -> [`core/`](core/)
- Explore the standard library -> [`std/`](std/)

## Troubleshooting

### Build fails

- Ensure all dependencies are installed
- Check LLVM version (must be v15)
- Use prebuild packages from Github Release

### Program does not run

- Make sure the binary was generated
- Check execution permissions:
```bash
chmod +x build/helloProgram
```

## Note

Lubex is an experimental project.
Expect bugs and incomplete features.
