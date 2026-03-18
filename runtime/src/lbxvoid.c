#include "lbxvoid.h"
#include "lbxmem.h"
#include "lbxlib.h"

_BI_Void* _BI_Void_init() {
    _BI_Void* voidVal = (_BI_Void*)_BI_malloc(sizeof(_BI_Void));
    voidVal->typeId = 0; // TODO: Add RTTI for all types
    return voidVal;
}

_BI_Void* _BI_Void_add(_BI_Void* a, _BI_Void* b) {
    UNUSED(a);
    UNUSED(b);

    _BI_Void* c = (_BI_Void*)_BI_malloc(sizeof(_BI_Void));
    return c;
}

_BI_Void* _BI_Void_subtract(_BI_Void* a, _BI_Void* b) {
    UNUSED(a);
    UNUSED(b);

    _BI_Void* c = (_BI_Void*)_BI_malloc(sizeof(_BI_Void));
    return c;
}

_BI_Void* _BI_Void_multiply(_BI_Void* a, _BI_Void* b) {
    UNUSED(a);
    UNUSED(b);

    _BI_Void* c = (_BI_Void*)_BI_malloc(sizeof(_BI_Void));
    return c;
}

_BI_Void* _BI_Void_divide(_BI_Void* a, _BI_Void* b) {
    UNUSED(a);
    UNUSED(b);

    _BI_Void* c = (_BI_Void*)_BI_malloc(sizeof(_BI_Void));
    return c;
}