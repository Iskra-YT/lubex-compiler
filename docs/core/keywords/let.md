# Let Keyword

**The `let` keyword is used to declare a variable in Lubex. It allows you to create a mutable variable that can be reassigned with a new value.**

## Syntax

```lubex
let variableName: Type = value;
```

## Description

The `let` keyword is used to declare a variable in Lubex. It allows you to create a mutable variable that can be reassigned with a new value. This is useful for situations where you need to store and modify data throughout the program.

## Usage

```lubex
module main;

class SimpleProgram {
    public static func main(): Number {
        let message: String = "Hello, World!";
        std.Console.write(message);
        message = "Welcome to Lubex!";
        std.Console.write(message);
        return 0;
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Variable declaration with type inference | Declares a variable with an inferred type based on the assigned value |
| Variable declaration with explicit type | Declares a variable with an explicitly specified type |

## Notes

- Variables declared with `let` can be reassigned to new values.
- The type of a variable can be explicitly specified or inferred from the assigned value.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `let` keyword for declaring mutable variables |
| lubex-r202605a01 | Modify | Update `let` keyword to support type inference |
| lubex-r202606a01 | Modify | Update example to use `std.Console.write` |

## See Also

- [Const Keyword](./const.md)