# Private Keyword

**The `private` keyword is used to define private members of a class in Lubex. Private members can only be accessed from within the class itself.**

## Syntax

```lubex
private func functionName(parameters): ReturnType {
    // function body
};
```

## Description

The `private` keyword is placed before the member declaration (field or method) to indicate that it is only accessible from within the class.

## Usage

```lubex
module main;

class Example {
    private func secretMethod(): Void {
        // secret implementation
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Private member | Defines a member that can only be accessed from within the class |

## Notes

Private members cannot be accessed using the dot notation from outside the class. They are only accessible within the class's own methods.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `private` keyword for defining private members |
| lubex-r202606a01 | Modify | Remove `->` from syntax |

## See also

- [Public Keyword](./public.md)
- [Func Keyword](./func.md)
- [Static Keyword](./static.md)
- [Internal Keyword](./internal.md)