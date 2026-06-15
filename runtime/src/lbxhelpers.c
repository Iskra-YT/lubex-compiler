#include <stdint.h>

static const char* hex = "0123456789ABCDEF";

void __R_ptr_to_hex(uintptr_t value, char* out) {
    char buffer[2 * sizeof(uintptr_t)];

    for (int i = (int)(sizeof(uintptr_t) * 2 - 1); i >= 0; --i) {
        buffer[i] = hex[value & 0xF];
        value >>= 4;
    }

    __R_memcpy(out, buffer, sizeof(buffer));
    out[sizeof(uintptr_t) * 2] = '\0';
}