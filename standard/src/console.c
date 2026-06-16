#include "console.h"

_BI_Void* _std_console_out(_BI_String* fmt) {
    if (!fmt || !fmt->data) return NULL;
    __R_print(fmt->data);
    _BI_Void* voidVal = (_BI_Void*)_BI_malloc(sizeof(_BI_Void));
    if (!voidVal) return NULL;
    return _BI_Void_init(voidVal);
}

_BI_Void* _std_console_outl(_BI_String* text) {
    if (!text || !text->data) return NULL;
    __R_print(text->data);
    __R_print("\n");
    _BI_Void* voidVal = (_BI_Void*)_BI_malloc(sizeof(_BI_Void));
    if (!voidVal) return NULL;
    return _BI_Void_init(voidVal);
}