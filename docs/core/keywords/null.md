# Null Keyword

**The `null` keyword represents a null value in Lubex. It can be assigned to nullable types.**

## Syntax

```lubex
let variable: Type? = null;
```

## Description

The `null` keyword is a literal that represents the absence of a value. It can only be assigned to variables of nullable types (types suffixed with `?`). Assigning `null` to a non-nullable variable will produce a compile-time error.

## Usage

```lubex
module main;

class Program {
    public static func entry(): Number {
        let x: Number? = null;
        let y: String? = null;
        return 0;
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Assignment to nullable type | Assigns null to a nullable variable |
| Assignment to non-nullable type | Compile-time error |

## Notes

- `null` can only be used with nullable types
- Use the null safety operators (`??`, `?:`, `?.`) to handle null values safely

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202606a01 | Create | Create `null` keyword for representing null values |

## See also

- [Nullable Type](../types/Nullable.md)
- [Let Keyword](./let.md)
