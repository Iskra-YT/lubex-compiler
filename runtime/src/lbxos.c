#include "lbxos.h"

void __OS_exit(int exitCode) {
    asm volatile(
        "syscall"
        :
        : "a"(0x3C),
          "D"(exitCode)
        : "rcx", "r11", "memory"
    );
}