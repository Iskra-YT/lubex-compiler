#include "lbxstr.h"
#include "lbxmem.h"

_BI_String* _BI_String_init(_BI_String* strVal, char* val) {
    if (!strVal || !val) return NULL;

    long len = __R_strlen(val);
    char* copy = (char*)_BI_malloc(len + 1);
    if (!copy) return NULL;
    __R_memcpy(copy, val, len + 1);

    strVal->data = copy;
    strVal->size = len;

    return strVal;
}