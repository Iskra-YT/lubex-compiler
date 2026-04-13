#ifndef LUBRTX_LIB_LBXINT_H
#define LUBRTX_LIB_LBXINT_H

#include "lbxtype.h"

typedef struct {
    _BI_TypeInfo* type;
    double value;
} _BI_Number;

_BI_Number* _BI_Number_init(_BI_Number* mem, double val);
_BI_Number* _BI_Number_add(_BI_Number* a, _BI_Number* b);
_BI_Number* _BI_Number_subtract(_BI_Number* a, _BI_Number* b);
_BI_Number* _BI_Number_multiply(_BI_Number* a, _BI_Number* b);
_BI_Number* _BI_Number_divide(_BI_Number* a, _BI_Number* b);

#endif // LUBRTX_LIB_LBXINT_H