#include "math.h"

_BI_Number* _std_math_pow(_BI_Number* base, _BI_Number* exponent) {
    if (!base || !exponent) return NULL;

    _BI_Number* result = (_BI_Number*)_BI_malloc(sizeof(_BI_Number));
    if (!result) return NULL;

    if (exponent->value == 0) {
        result->value = 1;
        return result;
    }

    double res = 1.0;
    double b = base->value;
    double e = exponent->value;

    while (e > 0) {
        if ((int)e % 2 == 1) {
            res *= b;
        }
        b *= b;
        e /= 2;
    }

    result->value = res;
    return result;
}