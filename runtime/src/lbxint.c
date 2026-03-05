#include "lbxint.h"
#include "lbxmem.h"

_BI_Int* _BI_Int_init(double val) {
    _BI_Int* intVal = (_BI_Int*)_BI_malloc(sizeof(_BI_Int));
    intVal->value = val;
    return intVal;
}

_BI_Int* _BI_Int_add(_BI_Int* a, _BI_Int* b) {
    _BI_Int* c = (_BI_Int*)_BI_malloc(sizeof(_BI_Int));
    c->value = a->value + b->value;
    return c;
}

_BI_Int* _BI_Int_subtract(_BI_Int* a, _BI_Int* b) {
    _BI_Int* c = (_BI_Int*)_BI_malloc(sizeof(_BI_Int));
    c->value = a->value - b->value;
    return c;
}

_BI_Int* _BI_Int_multiply(_BI_Int* a, _BI_Int* b) {
    _BI_Int* c = (_BI_Int*)_BI_malloc(sizeof(_BI_Int));
    c->value = a->value * b->value;
    return c;
}

_BI_Int* _BI_Int_divide(_BI_Int* a, _BI_Int* b) {
    _BI_Int* c = (_BI_Int*)_BI_malloc(sizeof(_BI_Int));
    c->value = a->value / b->value;
    return c;
}