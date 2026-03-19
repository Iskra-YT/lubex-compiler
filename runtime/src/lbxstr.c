#include "lbxstr.h"
#include "lbxmem.h"

_BI_String* _BI_String_init(char* val) {
    long len = __R_strlen(val);

    _BI_String* strVal = (_BI_String*)_BI_malloc(sizeof(_BI_String));

    char* copy = (char*)_BI_malloc(len + 1);
    __R_memcpy(copy, val, len + 1);

    strVal->data = copy;

    return strVal;
}