#ifndef LUBRTX_LIB_LBXOBJECT_H
#define LUBRTX_LIB_LBXOBJECT_H

#include "lbxtype.h"
#include "lbxstr.h"
#include "lbxmem.h"
#include <stdint.h>

typedef struct {
    _BI_TypeInfo* type;
} _BI_Object;

_BI_Object* _BI_Object_init(_BI_Object* obj);
_BI_String* _BI_Object_toString(_BI_Object* obj);

#endif // LUBRTX_LIB_LBXOBJECT_H