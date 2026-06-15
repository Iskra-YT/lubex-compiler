# Const Keyword

**The `const` keyword is used to declare a constant variable in Lubex. A constant variable is a variable whose value cannot be changed after it has been initialized.**

## Syntax

```lubex
const variableName: Type = value;
```

## Description

The `const` keyword is used to declare a constant variable in Lubex. A constant variable is a variable whose value cannot be changed after it has been initialized. This is useful for defining values that should remain constant throughout the program, such as mathematical constants or configuration values.

## Usage

```lubex
module main;

class Math {
    public static func areaOfCircle(radius: Number): Number {
        const PI: Number = 3.14159;
        return PI * radius * radius;
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Constant declaration with type inference | Declares a constant variable with an inferred type based on the assigned value |
| Constant declaration with explicit type | Declares a constant variable with an explicitly specified type |

## Notes

- Constant variables must be initialized at the time of declaration.
- The value of a constant variable cannot be changed after it has been initialized.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `const` keyword for declaring constant variables |
| lubex-r202605a01 | Modify | Update `const` keyword to support type inference |

## See Also

- [Let Keyword](./let.md)