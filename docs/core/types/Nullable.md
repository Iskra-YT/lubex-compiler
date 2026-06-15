# Nullable Types

**Nullable types allow variables to hold either a value of the underlying type or `null`.**

## Syntax

```lubex
let variableName: Type? = value;
```

## Description

A nullable type is denoted by appending `?` to a type name (e.g., `Number?`, `String?`). 
Variables of nullable types can hold a value of the base type or the special `null` value.
This enables safer handling of potentially missing values.

## Usage

```lubex
module main;

class Program {
    public static func entry(): Number {
        let x: Number? = null;
        let y: Number? = Number.init(42);

        return 0;
    };
};
```

## Null Safety Operators

Lubex provides several operators for working with nullable types:

| Operator | Name | Description |
|----------|------|-------------|
| `??` | Null check | Unwraps a nullable value, producing an error if null |
| `?:` | Null coalescing | Provides a default value when the left operand is null |
| `?.` | Safe navigation | Accesses a member safely, returning null if the object is null |

### Null Check (`??`)

```lubex
let unwrapped: Number = x ??;
```

Throws a runtime error if the value is `null`. The result is a non-nullable type.

### Null Coalescing (`?:`)

```lubex
let result: Number = x ?: Number.init(0);
```

Returns the left operand if it is not null; otherwise returns the right operand.

### Safe Navigation (`?.`)

```lubex
let length: Number? = str?.length;
```

Accesses a member of an object. If the object is null, the entire expression evaluates to null instead of throwing an error.

## Notes

- `null` can only be assigned to nullable types
- Using `.` (dot) on a nullable type will produce a compile-time error; use `?.` instead
- The `?:` operator requires both operands to have the same base type

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202606a01 | Create | Create nullable types and null safety operators |
