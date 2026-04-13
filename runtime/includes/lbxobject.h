#ifndef LUBRTX_LIB_LBXOBJECT_H
#define LUBRTX_LIB_LBXOBJECT_H

#include "lbxtype.h"

typedef struct {
    _BI_TypeInfo* type;
} _BI_Object;

_BI_Object* _BI_Object_init(_BI_Object* this);

#endif // LUBRTX_LIB_LBXOBJECT_H