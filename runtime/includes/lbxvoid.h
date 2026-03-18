#ifndef LUBRTX_LIB_LBXVOID_H
#define LUBRTX_LIB_LBXVOID_H

typedef struct {
    long typeId;
} _BI_Void;

_BI_Void* _BI_Void_init();
_BI_Void* _BI_Void_add(_BI_Void* a, _BI_Void* b);
_BI_Void* _BI_Void_subtract(_BI_Void* a, _BI_Void* b);
_BI_Void* _BI_Void_multiply(_BI_Void* a, _BI_Void* b);
_BI_Void* _BI_Void_divide(_BI_Void* a, _BI_Void* b);

#endif // LUBRTX_LIB_LBXVOID_H