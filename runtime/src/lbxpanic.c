#include "lbxpanic.h"
#include "lbxos.h"

void __R_panic(const char* message) {
    const char* prefix = "Panic: ";

    long prefixLen = 0;
    while (prefix[prefixLen]) prefixLen++;

    long msgLen = 0;
    while (message[msgLen]) msgLen++;

    __OS_output(prefix, prefixLen);
    __OS_output(message, msgLen);
    __OS_output("\n", 1);

    __OS_exit(1);
}
