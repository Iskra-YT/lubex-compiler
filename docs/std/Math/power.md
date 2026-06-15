# std.Math.power

**Calculates the value of a base raised to the power of an exponent.**

## Syntax

```lubex
std.Math.power(base: Number, exponent: Number): Number
```

## Parameters

| Parameter | Type | Description |
| --------- | ---- | ------ | 
| `base` | Number | The base value |
| `exponent` | Number | The exponent value |

## Returns
`Number` - The result of `base` raised to the power of `exponent`.

## Usage
```lubex
module main;

import std;

class MathExample {
    public static func entry(): Number {
        let result: Number = std.Math.power(2, 3);
        std.Console.writeLn("2^3 = " + result);
        return 0;
    };
};
```

**Output**:
```
2^3 = 8
```

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202606a01 | Create | Create `power` function for exponentiation |

## See also

- [Number Keyword](../../core/types/Number.md)