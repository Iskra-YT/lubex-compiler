#include "lbxio.h"
#include "lbxos.h"

void __R_print(const char* s) {
    long len = 0;
    while (s[len]) len++;

    __OS_output(s, len);
}