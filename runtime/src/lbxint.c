#include "lbxint.h"
#include "lbxmem.h"

_BI_Number* _BI_Number_init(_BI_Number* mem, double val) {
    mem->value = val;
    return mem;
}

_BI_Number* _BI_Number_add(_BI_Number* a, _BI_Number* b) {
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = a->value + b->value;
    return c;
}

_BI_Number* _BI_Number_subtract(_BI_Number* a, _BI_Number* b) {
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = a->value - b->value;
    return c;
}

_BI_Number* _BI_Number_multiply(_BI_Number* a, _BI_Number* b) {
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = a->value * b->value;
    return c;
}

_BI_Number* _BI_Number_divide(_BI_Number* a, _BI_Number* b) {
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = a->value / b->value;
    return c;
}