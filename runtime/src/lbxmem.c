#include "lbxmem.h"
#include "lbxos.h"

extern HEAP_BLOCK* __R_mainHeap;

HEAP_BLOCK* __R_cblock(HEAP_BLOCK* previous) {
    HEAP_BLOCK* newBlock =
        (HEAP_BLOCK*)__OS_mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (newBlock == (void*)-1) return NULL;

    __R_memset(newBlock, 0, 4096);

    if (previous) previous->next = newBlock;
    return newBlock;
}

HEAP_BLOCK* __R_malloc_init(void) {
    return __R_cblock(NULL);
}

void* _BI_malloc(long size) {
    int blocks_needed = (size + 7) / 8;
    HEAP_BLOCK* block = __R_mainHeap;

    while (block) {
        for (int i = 0; i < 8; i++) {
            uint64_t chunk = block->bitmap[i];

            if (~chunk == 0) continue;

            uint64_t free_bits = ~chunk;

            while (free_bits) {
                int bit = __builtin_ctzll(free_bits);
                int start = i * 64 + bit;

                if (start + blocks_needed > 503) {
                    free_bits &= free_bits - 1;
                    continue;
                }

                int ok = 1;
                for (int j = 0; j < blocks_needed; j++) {
                    int idx = start + j;
                    int ci = idx / 64;
                    int bi = idx % 64;

                    if (ci >= 8 || (block->bitmap[ci] & (1ULL << bi))) {
                        ok = 0;
                        break;
                    }
                }

                if (ok) {
                    for (int j = 0; j < blocks_needed; j++) {
                        int idx = start + j;
                        int ci = idx / 64;
                        int bi = idx % 64;

                        block->bitmap[ci] |= (1ULL << bi);
                    }

                    return (void*)(&block->data[start]);
                }

                free_bits &= free_bits - 1;
            }
        }

        if (!block->next) block = __R_cblock(block);
        else block = (HEAP_BLOCK*)block->next;
    }

    return NULL;
}

void* __R_memset(void* ptr, int value, unsigned long num) {
    asm volatile("rep stosb" : "+D"(ptr), "+c"(num) : "a"(value) : "memory");
    return ptr;
}

unsigned long __R_strlen(const char* str) {
    if (!str) return 0;

    const char* s = str;
    while (*s) {
        s++;
    }
    return (unsigned long)(s - str);
}

void* __R_memcpy(void* dest, const void* src, unsigned long n) {
    asm volatile("rep movsb" : "+D"(dest), "+S"(src), "+c"(n) : : "memory");
    return dest;
}