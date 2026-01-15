#ifndef IONET_CORE_TYPES_H
#define IONET_CORE_TYPES_H

#include <cstdint>
#include <string>
#include <vector>
#include <variant>

namespace ionet::core {

/// Supported data types for schema fields
enum class DataType {
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float32,
    Float64,
    Bitfield,
    String,
    Bytes
};

/// Byte ordering
enum class ByteOrder {
    Big,
    Little,
    Native
};

/// Universal value type for decoded/encoded fields
using Value = std::variant<
    std::monostate,           // Empty/unset
    int64_t,                  // All signed integers
    uint64_t,                 // All unsigned integers
    double,                   // All floats
    std::string,              // Strings
    std::vector<uint8_t>      // Raw bytes
>;

/// Get size in bytes for a data type
constexpr std::size_t dataTypeSize(DataType type) {
    switch (type) {
        case DataType::Int8:    return 1;
        case DataType::Int16:   return 2;
        case DataType::Int32:   return 4;
        case DataType::Int64:   return 8;
        case DataType::UInt8:   return 1;
        case DataType::UInt16:  return 2;
        case DataType::UInt32:  return 4;
        case DataType::UInt64:  return 8;
        case DataType::Float32: return 4;
        case DataType::Float64: return 8;
        case DataType::Bitfield: return 0; // Variable
        case DataType::String:  return 0;  // Variable
        case DataType::Bytes:   return 0;  // Variable
    }
    return 0;
}

/// Check if type is signed integer
constexpr bool isSigned(DataType type) {
    switch (type) {
        case DataType::Int8:
        case DataType::Int16:
        case DataType::Int32:
        case DataType::Int64:
            return true;
        default:
            return false;
    }
}

/// Check if type is unsigned integer
constexpr bool isUnsigned(DataType type) {
    switch (type) {
        case DataType::UInt8:
        case DataType::UInt16:
        case DataType::UInt32:
        case DataType::UInt64:
            return true;
        default:
            return false;
    }
}

/// Check if type is integer (signed or unsigned)
constexpr bool isInteger(DataType type) {
    return isSigned(type) || isUnsigned(type);
}

/// Check if type is floating point
constexpr bool isFloat(DataType type) {
    return type == DataType::Float32 || type == DataType::Float64;
}

/// Check if type is numeric
constexpr bool isNumeric(DataType type) {
    return isInteger(type) || isFloat(type);
}

/// Convert DataType to string
inline const char* dataTypeToString(DataType type) {
    switch (type) {
        case DataType::Int8:     return "int8";
        case DataType::Int16:    return "int16";
        case DataType::Int32:    return "int32";
        case DataType::Int64:    return "int64";
        case DataType::UInt8:    return "uint8";
        case DataType::UInt16:   return "uint16";
        case DataType::UInt32:   return "uint32";
        case DataType::UInt64:   return "uint64";
        case DataType::Float32:  return "float32";
        case DataType::Float64:  return "float64";
        case DataType::Bitfield: return "bitfield";
        case DataType::String:   return "string";
        case DataType::Bytes:    return "bytes";
    }
    return "unknown";
}

} // namespace ionet::core

#endif