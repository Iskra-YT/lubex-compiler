#include <stdint.h>
#include "lbxos.h"

void __OS_exit(int exitCode) {
    asm volatile(
        "syscall"
        :
        : "a"(OS_EXIT), "D"(exitCode)
        : "rcx", "r11", "memory"
    );
}

void* __OS_mmap(void* addr, long length, int prot, int flags, int fd, long offset) {
    long ret;
    register long r10 asm("r10") = offset;
    register long r8  asm("r8")  = flags;
    register long r9  asm("r9")  = fd;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(OS_MMAP), "D"(addr), "S"(length), "d"(prot), "r"(r8), "r"(r9), "r"(r10)
        : "rcx", "r11", "memory"
    );

    if (ret < 0) return (void*)-1;
    return (void*)ret;
}