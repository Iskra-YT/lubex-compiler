#ifndef LUBRTX_LIB_LBXINT_H
#define LUBRTX_LIB_LBXINT_H

typedef struct {
    double value;
} _BI_Number;

_BI_Number* _BI_Number_init(double val);
_BI_Number* _BI_Number_add(_BI_Number* a, _BI_Number* b);
_BI_Number* _BI_Number_subtract(_BI_Number* a, _BI_Number* b);
_BI_Number* _BI_Number_multiply(_BI_Number* a, _BI_Number* b);
_BI_Number* _BI_Number_divide(_BI_Number* a, _BI_Number* b);

#endif // LUBRTX_LIB_LBXINT_H