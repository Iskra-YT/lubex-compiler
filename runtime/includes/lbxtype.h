#ifndef LUBRTX_LIB_LBXTYPE_H
#define LUBRTX_LIB_LBXTYPE_H

typedef struct {
    unsigned __int128 typeId;
    struct _BI_TypeInfo* parent;
    unsigned long** vTable;
} _BI_TypeInfo;

#endif // LUBRTX_LIB_LBXTYPE_H