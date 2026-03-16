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

    register long r10 asm("r10") = flags;
    register long r8  asm("r8")  = fd;
    register long r9  asm("r9")  = offset;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(OS_MMAP), "D"(addr), "S"(length), "d"(prot), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory"
    );

    if (ret < 0) return (void*)-1;
    return (void*)ret;
}

long __OS_output(const char* buffer, long length) {
    long ret;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(OS_WRITE), "D"(1), "S"(buffer), "d"(length)
        : "rcx", "r11", "memory"
    );

    return ret;
}

long __OS_input(char* buffer, long length) {
    long ret;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(OS_READ), "D"(0), "S"(buffer), "d"(length)
        : "rcx", "r11", "memory"
    );

    return ret;
}