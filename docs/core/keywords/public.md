# Public Keyword

**The `public` keyword is used to define public members of a class in Lubex. Public members can be accessed from outside the class.**

## Syntax

```lubex
public func functionName(parameters): ReturnType {
    // function body
};
```

## Description

The `public` keyword is placed before the member declaration (field or method) to indicate that it is accessible from outside the class.

## Usage

```lubex
module main;

class Point {
    let x: Number;
    let y: Number;

    public func move(dx: Number, dy: Number): Void {
        this.x = this.x + dx;
        this.y = this.y + dy;
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Public member | Defines a member that can be accessed from outside the class |

## Notes

Public members can be accessed using the dot notation on an instance of the class.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `public` keyword for defining public members |
| lubex-r202606a01 | Modify | Remove `->` from syntax |

## See also

- [Private Keyword](./private.md)
- [Func Keyword](./func.md)
- [Static Keyword](./static.md)
- [Internal Keyword](./internal.md)