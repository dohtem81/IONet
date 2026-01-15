#ifndef IONET_CORE_ENDIAN_H
#define IONET_CORE_ENDIAN_H

#include "Types.h"
#include <bit>
#include <concepts>
#include <cstring>
#include <type_traits>

namespace ionet::core::endian {

constexpr ByteOrder getNativeByteOrder() {
        return (std::endian::native == std::endian::little) ? ByteOrder::Little : ByteOrder::Big;
    }

constexpr bool needsSwap(ByteOrder from) {
        constexpr bool isLittleEndian = (std::endian::native == std::endian::little);
        return (from == ByteOrder::Big && isLittleEndian) ||
               (from == ByteOrder::Little && !isLittleEndian);

    }

template<std::integral T>
constexpr T byteSwap(T value) {
        if constexpr (sizeof(T) == 1) {
            return value;
        } else if constexpr (sizeof(T) == 2) {
            using U = std::make_unsigned_t<T>;
            U uval = static_cast<U>(value);
            uval = static_cast<U>((uval >> 8) | (uval << 8));
            return static_cast<T>(uval);
        } else if constexpr (sizeof(T) == 4) {
            using U = std::make_unsigned_t<T>;
            U uval = static_cast<U>(value);
            uval = ((uval >> 24) & 0x000000FF) |
                   ((uval >> 8)  & 0x0000FF00) |
                   ((uval << 8)  & 0x00FF0000) |
                   ((uval << 24) & 0xFF000000);
            return static_cast<T>(uval);
        } else if constexpr (sizeof(T) == 8) {
            using U = std::make_unsigned_t<T>;
            U uval = static_cast<U>(value);
            uval = ((uval >> 56) & 0x00000000000000FFULL) |
                   ((uval >> 40) & 0x000000000000FF00ULL) |
                   ((uval >> 24) & 0x0000000000FF0000ULL) |
                   ((uval >> 8)  & 0x00000000FF000000ULL) |
                   ((uval << 8)  & 0x000000FF00000000ULL) |
                   ((uval << 24) & 0x0000FF0000000000ULL) |
                   ((uval << 40) & 0x00FF000000000000ULL) |
                   ((uval << 56) & 0xFF00000000000000ULL);
            return static_cast<T>(uval);
        }
    }

    template<typename T>
T convert(T value, ByteOrder from) {
        if (from == ByteOrder::Native) {
            return value;
        }

        bool doSwap = needsSwap(from);

        if constexpr (std::is_floating_point_v<T>) {
            if (doSwap) {
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
        return doSwap ? byteSwap(value) : value;
        }
    }

} // namespace ionet::core::endian
#endif