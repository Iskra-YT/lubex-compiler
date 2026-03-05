#ifndef LUBRTX_LIB_LBXINT_H
#define LUBRTX_LIB_LBXINT_H

typedef struct {
    double value;
} _BI_Int;

_BI_Int* _BI_Int_init(double val);
_BI_Int* _BI_Int_add(_BI_Int* a, _BI_Int* b);
_BI_Int* _BI_Int_subtract(_BI_Int* a, _BI_Int* b);
_BI_Int* _BI_Int_multiply(_BI_Int* a, _BI_Int* b);
_BI_Int* _BI_Int_divide(_BI_Int* a, _BI_Int* b);

#endif // LUBRTX_LIB_LBXINT_H