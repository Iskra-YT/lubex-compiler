#ifndef LUBRTX_LIB_LBXSTRING_H
#define LUBRTX_LIB_LBXSTRING_H

typedef struct {
    char* data;
    long size;
} _BI_String;

_BI_String* _BI_String_init(_BI_String* strVal, char* val);

#endif // LUBRTX_LIB_LBXSTRING_H