# std.Console.writeLn

**Outputs value to the standard console with a trailing newline.**

## Syntax

```lubex
std.Console.writeLn(text: String): Void
```

## Parameters

| Parameter | Type | Description |
| --------- | ---- | ------ | 
| `text` | String | The text to be written to the standard output |

## Returns
`Void` - This function does not return any value.

## Usage
```lubex
module main;

import std;

class Hello {
    public static func entry(): Number {
        std.Console.writeLn("Hello, World!");
        return 0;
    };
};
```

**Output**:
```
Hello, World!
```

## Notes
- The function automatically appends a newline character (`\n`) after the output.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202606a01 | Create | Create `writeLn` function for printing with newline |

## See also

- [std.Console.write](./write.md)