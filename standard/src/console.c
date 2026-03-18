#include "console.h"

_BI_Void* _std_console_out(_BI_Int* fmt) {
    char val[] = { (char)fmt->value, '\0' };
    __R_print(val);
    return _BI_Void_init();
}