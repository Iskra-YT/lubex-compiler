# Class Keyword

**The `class` keyword is used to define a class in Lubex. A class is a blueprint for creating objects that encapsulate data and behavior.**

## Syntax

```lubex
class ClassName {
    // class body
};
```

## Description

The `class` keyword is followed by the class name and a block of code enclosed in curly braces. The class body can contain fields (variables) and methods (functions) that define the properties and behaviors of the class.

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
| Class definition | Defines a new class with specified fields and methods |

## Notes

- Classes can be instantiated to create objects that hold specific data and can perform actions defined by the class methods.
- The `this` keyword is used within class methods to refer to the current instance of the class.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create `class` keyword for defining classes |
| lubex-r202606a01 | Modify | Remove `->` from syntax |

## See also

- [Func Keyword](./func.md)
- [Public Keyword](./public.md)
- [This Keyword](./this.md)
- [Extends Keyword](./extends.md)