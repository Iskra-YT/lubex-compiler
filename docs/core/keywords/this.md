# This Keyword

**The `this` keyword is used within class methods to refer to the current instance of the class. It allows access to the instance's fields and methods.**

## Syntax

```lubex
this.fieldName
this.methodName(arguments)
```

## Description

The `this` keyword is a reference to the current object instance. It is commonly used to access fields and methods of the class from within its methods.

## Usage

```lubex
module main;

class Point -> {
    let x: Int;
    let y: Int;

    public func move(dx: Int, dy: Int): Void -> {
        this.x = this.x + dx;
        this.y = this.y + dy;
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Accessing fields | `this.fieldName` allows access to the instance's fields |
| Calling methods | `this.methodName(arguments)` allows calling other methods of the instance |

## Notes

In static methods, the `this` keyword cannot be used since there is no instance context.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202605a01 | Create | Create `this` keyword for referring to the current instance in class methods |

## See also

- [Class Keyword](./class.md)
- [Func Keyword](./func.md)
- [Public Keyword](./public.md)