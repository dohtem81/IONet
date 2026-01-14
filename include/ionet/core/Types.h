#ifndef IONET_TYPES_H
#define IONET_TYPES_H

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <optional>

namespace ionet::core {

enum class DataType {
    Int8, Int16, Int32, Int64,
    UInt8, UInt16, UInt32, UInt64,
    Float32, Float64,
    Bitfield,
    String,
    Array
};

enum class ByteOrder {
    Little,
    Big,
    Native
};

// Size in bytes for each type
constexpr std::size_t dataTypeSize(DataType type) {
    switch (type) {
        case DataType::Int8:
        case DataType::UInt8:    return 1;
        case DataType::Int16:
        case DataType::UInt16:   return 2;
        case DataType::Int32:
        case DataType::UInt32:
        case DataType::Float32:  return 4;
        case DataType::Int64:
        case DataType::UInt64:
        case DataType::Float64:  return 8;
        default:                 return 0; // Variable size
    }
}

// Runtime value holder
using Value = std::variant<
    std::monostate,
    int64_t,
    uint64_t,
    double,
    std::string,
    std::vector<uint8_t>
>;

} // namespace ionet::core

#endif