#include "console.h"

_BI_Void* _std_console_out(_BI_String* fmt) {
    __R_print(fmt->data);
    return _BI_Void_init();
}