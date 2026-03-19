# Int

**Represents a float value.**

## Description

The `Int` type is used to store numbers. 
The size and range of `Int` is 64 bits and corresponds to a **64-bit IEEE 754 floating-point number**.

## Syntax
```lubex
let x: Int = 42;
let y: Int = 12;
```

## Usage
Use `Int` for general-purpose floating-point arithmetic number.

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