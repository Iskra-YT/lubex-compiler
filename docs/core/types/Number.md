# Number

**Represents a float value.**

## Description

The `Number` type is used to store numbers. 
The size and range of `Number` is 64 bits and corresponds to a **64-bit IEEE 754 floating-point number**.

## Syntax
```lubex
let x: Number = 42;
let y: Number = 12.5;
```

## Usage
Use `Number` for general-purpose floating-point arithmetic.

## Operations

| Operation | Example | Description |
| --------- | ------- | ----------- |
| Addition  | `a + b` | Adds two numbers |
| Division  | `a / b` | Divides two numbers |
| Multiplication | `a * b` | Multiplies two numbers |
| Subtraction | `a - b` | Subtracts two numbers |

## Notes
- Overflow behavior is **undefined** and may wrap around or cause a runtime error.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `Int` type as floating-point number |
| lubex-r202605a01 | Modify | Rename `Int` type to `Number` to better reflect its floating-point nature |