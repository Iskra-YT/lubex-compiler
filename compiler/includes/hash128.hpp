#ifndef HASH128_LUBEX_H
#define HASH128_LUBEX_H

#include <array>
#include <cstdint>
#include <string>
#include <llvm/ADT/APInt.h>

struct Hash128 {
    std::array<uint8_t, 16> bytes;
};

inline Hash128 hash128(const std::string& str) {
    uint64_t h1 = 1469598103934665603ULL;
    uint64_t h2 = 1099511628211ULL;

    for (char c : str) {
        h1 ^= static_cast<uint8_t>(c);
        h1 *= 1099511628211ULL;

        h2 ^= static_cast<uint8_t>(c) + 0x9e3779b97f4a7c15ULL;
        h2 *= 14029467366897019727ULL;
    }

    Hash128 out;

    for (int i = 0; i < 8; i++) {
        out.bytes[i] = (h1 >> (i * 8)) & 0xFF;
        out.bytes[i + 8] = (h2 >> (i * 8)) & 0xFF;
    }

    return out;
}

inline llvm::APInt toAPInt(const Hash128& h) {
    llvm::APInt value(128, 0);

    for (int i = 0; i < 16; i++) {
        value = (value << 8) | h.bytes[i];
    }

    return value;
}

#endif // HASH128_LUBEX_H