#include "lbxvoid.h"
#include "lbxmem.h"
#include "lbxlib.h"

_BI_Void* _BI_Void_init() {
    _BI_Void* voidVal = (_BI_Void*)_BI_malloc(sizeof(_BI_Void));
    voidVal->typeId = 0; // TODO: Add RTTI for all types
    return voidVal;
}