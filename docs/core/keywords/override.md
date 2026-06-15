# Override Keyword

**The `override` keyword is used to indicate that a method in a subclass is intended to override a method with the same signature in its base class.**

## Syntax

```lubex
public override func methodName(parameters): ReturnType {
    // overridden implementation
};
```

## Description

The `override` keyword is placed before the `func` keyword to explicitly state that the method overrides a method from a parent class. This helps prevent accidental overrides and ensures that the method signature matches a method in the base class.

## Usage

```lubex
module main;

class Shape {
    public func draw(): Void {
        // generic draw
    };
};

class Circle extends Shape {
    public override func draw(): Void {
        // draw a circle
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Method override | Explicitly marks a method as overriding a base class method |

## Notes

- The `override` keyword is required when overriding a method from a base class.
- The base class must have a method with the same name and parameter types.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202606a01 | Create | Create `override` keyword for method overriding |

## See also

- [Extends Keyword](./extends.md)
- [Func Keyword](./func.md)