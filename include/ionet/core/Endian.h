#ifndef IONET_ENDIAN_H
#define IONET_ENDIAN_H

#include "Types.h"
#include <bit>
#include <concepts>

namespace ionet::core::endian {

template<std::integral T>
constexpr T byteSwap(T value) {
    if constexpr (sizeof(T) == 1) {
        return value;
    } else if constexpr (sizeof(T) == 2) {
        return static_cast<T>((value >> 8) | (value << 8));
    } else if constexpr (sizeof(T) == 4) {
        return static_cast<T>(
            ((value >> 24) & 0x000000FF) |
            ((value >> 8)  & 0x0000FF00) |
            ((value << 8)  & 0x00FF0000) |
            ((value << 24) & 0xFF000000)
        );
    } else if constexpr (sizeof(T) == 8) {
        return static_cast<T>(
            ((value >> 56) & 0x00000000000000FFULL) |
            ((value >> 40) & 0x000000000000FF00ULL) |
            ((value >> 24) & 0x0000000000FF0000ULL) |
            ((value >> 8)  & 0x00000000FF000000ULL) |
            ((value << 8)  & 0x000000FF00000000ULL) |
            ((value << 24) & 0x0000FF0000000000ULL) |
            ((value << 40) & 0x00FF000000000000ULL) |
            ((value << 56) & 0xFF00000000000000ULL)
        );
    }
}

template<typename T>
T convert(T value, ByteOrder from) {
    constexpr bool isLittleEndian = (std::endian::native == std::endian::little);
    
    if (from == ByteOrder::Native) {
        return value;
    }
    
    bool needsSwap = (from == ByteOrder::Big && isLittleEndian) ||
                     (from == ByteOrder::Little && !isLittleEndian);
    
    if constexpr (std::is_floating_point_v<T>) {
        if (needsSwap) {
            // Swap via integer representation
            if constexpr (sizeof(T) == 4) {
                uint32_t tmp;
                std::memcpy(&tmp, &value, sizeof(T));
                tmp = byteSwap(tmp);
                std::memcpy(&value, &tmp, sizeof(T));
            } else {
                uint64_t tmp;
                std::memcpy(&tmp, &value, sizeof(T));
                tmp = byteSwap(tmp);
                std::memcpy(&value, &tmp, sizeof(T));
            }
        }
        return value;
    } else {
        return needsSwap ? byteSwap(value) : value;
    }
}

} // namespace ionet::core::endian

#endif