# Func Keyword

**The `func` keyword is used to define a function in Lubex. It allows you to create reusable blocks of code that can be called with different arguments.**

## Syntax

```lubex
func functionName(parameters): ReturnType {
    // function body
};
```

## Description

The `func` keyword is followed by the function name, a list of parameters (if any), and the return type. The function body is enclosed in curly braces.

## Usage

```lubex
module main;

class Math {
    public static func add(a: Number, b: Number): Number {
        return a + b;
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Function with parameters | Defines a function that takes parameters and can be called with arguments |
| Function without parameters | Defines a function that does not take any parameters |

## Notes

Functions can only be defined inside a class. They can be called using the class name and the function name, or through an instance of the class.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `func` keyword for defining functions |
| lubex-r202606a01 | Modify | Remove `->` from syntax and update `Int` to `Number` |

## See also

- [Public Keyword](./public.md)
- [Static Keyword](./static.md)
- [Return Keyword](./return.md)
- [Override Keyword](./override.md)