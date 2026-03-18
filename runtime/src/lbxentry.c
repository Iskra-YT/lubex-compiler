#include "lbxos.h"
#include "lbxmem.h"

extern int main();

HEAP_BLOCK* __R_mainHeap;

void _start() {
    __R_mainHeap = __R_malloc_init();
    int exitCode = main();
    __OS_exit(exitCode);
}