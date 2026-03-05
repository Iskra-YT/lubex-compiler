#include "lbxmem.h"
#include "lbxos.h"

#define SET_BIT(bitmap, bit) ((bitmap)[(bit)/8] |= (1 << ((bit)%8)))
#define GET_BIT(bitmap, bit) (((bitmap)[(bit)/8] & (1 << ((bit)%8))) != 0)

extern HEAP_BLOCK* mainHeap;

HEAP_BLOCK* __R_cblock(HEAP_BLOCK* previous) {
    HEAP_BLOCK* newBlock = (HEAP_BLOCK*)__OS_mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
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
    HEAP_BLOCK* block = mainHeap;

    while (block) {
        int start = -1;
        int free_count = 0;

        for (int i = 0; i < 63*8; i++) {
            if (!GET_BIT(block->bitmap, i)) {
                if (start == -1) start = i;
                free_count++;
                if (free_count == blocks_needed) break;
            } else {
                start = -1;
                free_count = 0;
            }
        }

        if (free_count >= blocks_needed) {
            for (int i = start; i < start + blocks_needed; i++) SET_BIT(block->bitmap, i);     
            return (void*)(&block->data[start]);
        }

        if (!block->next) block = __R_cblock(block);
        else block = (HEAP_BLOCK*)block->next;
    }

    return NULL;
}

void* __R_memset(void* ptr, int value, unsigned long num) {
    asm volatile (
        "rep stosb"
        : "+D" (ptr), "+c" (num)
        : "a" (value)
        : "memory"
    );
    return ptr;
}