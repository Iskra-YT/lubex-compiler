#include "lbxos.h"
#include "lbxmem.h"

extern int main();

HEAP_BLOCK* mainHeap;

void _start() {
    mainHeap = __R_malloc_init();
    int exitCode = main();
    __OS_exit(exitCode);
}