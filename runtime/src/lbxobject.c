#include "lbxobject.h"

_BI_Object* _BI_Object_init(_BI_Object* this) {
    return this;
}

extern _BI_TypeInfo _T_BI_String;

_BI_String* _BI_Object_toString(_BI_Object* self) {
    static const char hex[] = "0123456789abcdef";

    uintptr_t addr = (uintptr_t)self;

    char buffer[64];
    char* p = buffer;

    const char* prefix = "Object@";

    while (*prefix) {
        *p++ = *prefix++;
    }

    int started = 0;

    for (int i = (sizeof(uintptr_t) * 2) - 1; i >= 0; --i) {
        unsigned nibble = (addr >> (i * 4)) & 0xF;

        if (nibble || started || i == 0) {
            started = 1;
            *p++ = hex[nibble];
        }
    }

    *p = '\0';

    _BI_String* str = _BI_malloc(sizeof(_BI_String));
    if (!str) return NULL;
    str->type = &_T_BI_String;
    str = _BI_String_init(str, buffer);
    return str;
}