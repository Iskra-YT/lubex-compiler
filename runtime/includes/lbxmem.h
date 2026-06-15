#ifndef LUBRTX_LIB_LBXMEM_H
#define LUBRTX_LIB_LBXMEM_H

#ifndef NULL
#define NULL ((void*)0)
#endif

#include <stdint.h>

typedef struct HEAP_BLOCK {
    struct HEAP_BLOCK* next;
    uint64_t bitmap[8];
    uint64_t data[503];
} __attribute__((packed)) HEAP_BLOCK;

HEAP_BLOCK* __R_malloc_init(void);
void* _BI_malloc(long size);

void* __R_memset(void* ptr, int value, unsigned long num);
unsigned long __R_strlen(const char* str);
void* __R_memcpy(void* dest, const void* src, unsigned long n);

#endif // LUBRTX_LIB_LBXMEM_H