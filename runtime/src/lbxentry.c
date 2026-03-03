#include "lbxos.h"

extern int main();

void _start() {
    int exitCode = main();
    __OS_exit(exitCode);
}