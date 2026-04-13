# Module Keyword

**The `module` keyword is used to define a module in Lubex. A module is a container for organizing code and can contain classes, functions, and other declarations.**

## Syntax

```lubex
module ModuleName;
```

## Description

The `module` keyword is followed by the module name and a semicolon. The module serves as a namespace for the code contained within it, allowing for better organization and preventing naming conflicts.

## Usage

```lubex
module main;

class Point -> {
    let x: Int;
    let y: Int;
};

```

## Behavior

| Case | Description |
| ---- | ----------- |
| Module definition | Defines a new module that can contain classes, functions, and other declarations |

## Notes

- Modules help in organizing code and managing namespaces.
- The module name should be unique to avoid conflicts with other modules.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `module` keyword for defining modules |

## See also

- [Class Keyword](./class.md)
- [Func Keyword](./func.md)
- [Import Keyword](./import.md)