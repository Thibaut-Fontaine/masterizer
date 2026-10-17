#pragma once

#include <cstdint>

namespace masterizer::io {

inline std::uint16_t readU16(const unsigned char* p) {
    return static_cast<std::uint16_t>(p[0] | (p[1] << 8));
}

inline std::uint32_t readU32(const unsigned char* p) {
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
}

inline void writeU32(unsigned char* p, std::uint32_t value) {
    p[0] = static_cast<unsigned char>(value & 0xFF);
    p[1] = static_cast<unsigned char>((value >> 8) & 0xFF);
    p[2] = static_cast<unsigned char>((value >> 16) & 0xFF);
    p[3] = static_cast<unsigned char>((value >> 24) & 0xFF);
}

/// Signed 24-bit little-endian sample, sign-extended.
inline std::int32_t readS24(const unsigned char* p) {
    std::int32_t value = static_cast<std::int32_t>(p[0]) |
                         (static_cast<std::int32_t>(p[1]) << 8) |
                         (static_cast<std::int32_t>(p[2]) << 16);
    if ((value & 0x800000) != 0) {
        value |= ~0xFFFFFF;
    }
    return value;
}

}  // namespace masterizer::io
