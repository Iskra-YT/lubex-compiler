#ifndef LUBRTX_LIB_LBXVOID_H
#define LUBRTX_LIB_LBXVOID_H

#include "lbxtype.h"

typedef struct {
    _BI_TypeInfo* type;
} _BI_Void;

_BI_Void* _BI_Void_init(_BI_Void* voidVal);

#endif // LUBRTX_LIB_LBXVOID_H