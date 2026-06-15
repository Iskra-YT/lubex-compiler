# Extends Keyword

**The `extends` keyword is used to define class inheritance in Lubex. It allows a class to inherit fields and methods from another class.**

## Syntax

```lubex
class SubClass extends BaseClass {
    // class body
};
```

## Description

The `extends` keyword is used in a class declaration to specify its base class. The subclass inherits all public and internal members of the base class.

## Usage

```lubex
module main;

class Animal {
    public func makeSound(): Void {
        // generic sound
    };
};

class Dog extends Animal {
    public override func makeSound(): Void {
        std.Console.write("Woof!");
    };
};
```

## Behavior

| Case | Description |
| ---- | ----------- |
| Class inheritance | Defines a class that inherits from another class |

## Notes

- Lubex supports single inheritance.
- Use the `override` keyword when redefining a method from the base class.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202606a01 | Create | Create `extends` keyword for class inheritance |

## See also

- [Class Keyword](./class.md)
- [Override Keyword](./override.md)