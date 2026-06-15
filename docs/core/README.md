# Lubex Language Reference

This section describes the Lubex programming language — its syntax, type system, and keywords.

## Types

| Type | Description | Documentation |
|------|-------------|---------------|
| `Number` | 64-bit IEEE 754 floating-point number | [Number](types/Number.md) |
| `String` | Sequence of characters | [String](types/String.md) |
| `Void` | Represents no value | — |
| `Object` | Base type for all classes | — |
| `Null` | Represents a null value | — |
| `T?` | Nullable variant of type T | [Nullable](types/Nullable.md) |

## Keywords

| Keyword | Description | Documentation |
|---------|-------------|---------------|
| `class` | Define a class | [class](keywords/class.md) |
| `const` | Declare a constant | [const](keywords/const.md) |
| `extends` | Inherit from a class | [extends](keywords/extends.md) |
| `func` | Define a function | [func](keywords/func.md) |
| `import` | Import a module | [import](keywords/import.md) |
| `internal` | Internal visibility | [internal](keywords/internal.md) |
| `let` | Declare a variable | [let](keywords/let.md) |
| `module` | Define a module | [module](keywords/module.md) |
| `override` | Override a method | [override](keywords/override.md) |
| `private` | Private visibility | [private](keywords/private.md) |
| `public` | Public visibility | [public](keywords/public.md) |
| `return` | Return from a function | [return](keywords/return.md) |
| `static` | Static function | [static](keywords/static.md) |
| `this` | Current instance reference | [this](keywords/this.md) |

## Operators

### Arithmetic

| Operator | Description |
|----------|-------------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division |

### Null Safety

| Operator | Description |
|----------|-------------|
| `??` | Null check (unwrap) |
| `?:` | Null coalescing (default value) |
| `?.` | Safe navigation |

### Assignment

| Operator | Description |
|----------|-------------|
| `=` | Assignment |

## Attributes

Attributes provide metadata for declarations:

```lubex
@mangle(symbolName)
class Foo { };
```

The `@mangle` attribute allows specifying a custom linker symbol name.

## See Also

- [Getting Started](../getting-started.md)
- [Standard Library](../std/README.md)
