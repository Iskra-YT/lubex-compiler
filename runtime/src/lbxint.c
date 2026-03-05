#include "lbxint.h"
#include "lbxmem.h"

_BI_Int* _BI_Int_init(double val) {
    _BI_Int* intVal = (_BI_Int*)_BI_malloc(sizeof(_BI_Int));
    intVal->value = val;
    return intVal;
}