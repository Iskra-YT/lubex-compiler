# Internal Keyword

**The `internal` keyword is used to define internal members of a class in Lubex. Internal members can be accessed from within the same module.**

## Syntax

```lubex
internal func functionName(parameters): ReturnType {
    // function body
};
```

## Description

The `internal` keyword is placed before the member declaration (field or method) to indicate that it is accessible from within the same module, but not from outside.

## Usage

```lubex
module myModule;

class Example {
    internal func moduleMethod(): Void {
        // internal implementation
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Internal member | Defines a member that can be accessed from within the same module |

## Notes

Internal visibility is the default if no other visibility keyword is specified.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202606a01 | Create | Create `internal` keyword for defining internal members |

## See also

- [Public Keyword](./public.md)
- [Private Keyword](./private.md)
- [Module Keyword](./module.md)