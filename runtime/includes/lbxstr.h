#ifndef LUBRTX_LIB_LBXSTRING_H
#define LUBRTX_LIB_LBXSTRING_H

typedef struct {
    char* data;
} _BI_String;

_BI_String* _BI_String_init(char* val);

#endif // LUBRTX_LIB_LBXSTRING_H