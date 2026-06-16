#include "lbxint.h"
#include "lbxstr.h"
#include "lbxmem.h"

_BI_Number* _BI_Number_init(_BI_Number* mem, double val) {
    if (!mem) return NULL;
    mem->value = val;
    return mem;
}

_BI_Number* _BI_Number_add(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = a->value + b->value;
    return c;
}

_BI_Number* _BI_Number_subtract(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = a->value - b->value;
    return c;
}

_BI_Number* _BI_Number_multiply(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = a->value * b->value;
    return c;
}

_BI_Number* _BI_Number_divide(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = a->value / b->value;
    return c;
}

_BI_Number* _BI_Number_equals(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = (a->value == b->value) ? 1.0 : 0.0;
    return c;
}

_BI_Number* _BI_Number_notEquals(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = (a->value != b->value) ? 1.0 : 0.0;
    return c;
}

_BI_Number* _BI_Number_lessThan(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = (a->value < b->value) ? 1.0 : 0.0;
    return c;
}

_BI_Number* _BI_Number_greaterThan(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = (a->value > b->value) ? 1.0 : 0.0;
    return c;
}

_BI_Number* _BI_Number_lessOrEqual(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = (a->value <= b->value) ? 1.0 : 0.0;
    return c;
}

_BI_Number* _BI_Number_greaterOrEqual(_BI_Number* a, _BI_Number* b) {
    if (!a || !b) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = (a->value >= b->value) ? 1.0 : 0.0;
    return c;
}

_BI_Number* _BI_Number_logicalNot(_BI_Number* a) {
    if (!a) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    c->value = (a->value == 0.0) ? 1.0 : 0.0;
    return c;
}

_BI_Number* _BI_Number_bitwiseNot(_BI_Number* a) {
    if (!a) return NULL;
    _BI_Number* c = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!c) return NULL;
    int64_t bits = (int64_t)a->value;
    c->value = (double)(~bits);
    return c;
}

_BI_String* _BI_Number_toString(_BI_Number* self) {
    if (!self) return NULL;

    char buffer[64];
    int written = 0;

    double val = self->value;
    if (val < 0) {
        buffer[written++] = '-';
        val = -val;
    }

    long long intPart = (long long)val;
    double fracPart = val - (double)intPart;

    char tmp[32];
    int tmpLen = 0;
    if (intPart == 0) {
        tmp[tmpLen++] = '0';
    } else {
        while (intPart > 0) {
            tmp[tmpLen++] = (char)('0' + (intPart % 10));
            intPart /= 10;
        }
    }
    for (int i = tmpLen - 1; i >= 0; i--) {
        buffer[written++] = tmp[i];
    }

    if (fracPart > 0.0) {
        buffer[written++] = '.';
        for (int i = 0; i < 6; i++) {
            fracPart *= 10.0;
            int digit = (int)fracPart;
            buffer[written++] = (char)('0' + digit);
            fracPart -= digit;
            if (fracPart < 1e-12) break;
        }
    }

    buffer[written] = '\0';

    _BI_String* str = (_BI_String*)_BI_malloc(sizeof(_BI_String));
    if (!str) return NULL;
    str = _BI_String_init(str, buffer);
    return str;
}