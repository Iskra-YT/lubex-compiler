# Static Keyword

**The `static` keyword is used to define static functions of a class in Lubex. Static functions can be called without creating an instance of the class.**

## Syntax

```lubex
static func functionName(parameters): ReturnType {
    // function body
};
```

## Description

The `static` keyword is placed before the `func` keyword when defining a function inside a class. This indicates that the function is static and can be called using the class name without needing to create an instance of the class.

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
| Static function | Defines a function that can be called without an instance of the class |

## Notes

Static functions can only access static fields of the class. They cannot access instance fields.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `static` keyword for defining static functions |
| lubex-r202606a01 | Modify | Remove `->` from syntax and update `Int` to `Number` |

## See also

- [Func Keyword](./func.md)
- [Public Keyword](./public.md)
- [Return Keyword](./return.md)