# Import Keyword

**The `import` keyword is used to include external modules or libraries into the current module in Lubex. This allows you to use the functionality defined in those modules without having to rewrite code.**

## Syntax

```lubex
import ModuleName;
```

## Description

The `import` keyword is followed by the name of the module you want to include, and a semicolon. Once imported, you can access the classes, functions, and other declarations defined in that module.

## Usage

```lubex
module main;

import std;

class HelloWorldProgram {
    public static func main(): Number {
        std.Console.write("Hello, World!");
        return 0;
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Import statement | Includes the specified module and allows access to its contents |

## Notes

- The module being imported must be available in the project or library path.
- Path resolution for imported modules is based on the project structure and configuration.

## Changelog
| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `import` keyword for including external modules |
| lubex-r202606a01 | Modify | Update example to use `std.Console.write` and `Number` |

## See also

- [Module Keyword](./module.md)
- [Class Keyword](./class.md)
- [Func Keyword](./func.md)