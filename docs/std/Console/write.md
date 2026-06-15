# std.Console.write

**Outputs value to the standard console.**

## Syntax

```lubex
std.Console.write(text: String): Void
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
        std.Console.write("Hello, World!\n");
        return 0;
    };
};
```

**Output**:
```
Hello, World!
```

## Notes
- The function **does not** automatically append a newline.

## Changelog

| Version | Action | Description |
| ------- | ------ | ----------- |
| lubex-r202603a01 | Create | Create function for printing ASCII value into standard output |
| lubex-r202605a01 | Modify | Change `Number` type format parameter into `String` type format parameter | 
| lubex-r202605a01 | Modify | Change function name from `out` into `write` |