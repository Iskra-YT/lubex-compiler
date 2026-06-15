# Return keyword

**The `return` keyword is used to exit a function and optionally return a value to the caller. It can be used in any function, including methods and constructors.**

## Syntax

```lubex
return expression;
```

## Description

The `return` statement immediately terminates the execution of the current function and returns control to the caller.

It can only be used inside a function body.

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
| `return` with an expression | Evaluates the expression and returns its value to the caller |
| `return` without an expression | Exits the function without returning a value (used in `Void` functions) |

## Notes

- Code after `return` is not executed
- Must match the function return type

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `return` keyword for exiting functions and returning values |
| lubex-r202606a01 | Modify | Remove `->` from syntax and update `Int` to `Number` |

## See also

- [Func Keyword](./func.md)
- [Public Keyword](./public.md)
- [Static Keyword](./static.md)