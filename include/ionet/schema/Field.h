#ifndef IONET_SCHEMA_FIELD_H
#define IONET_SCHEMA_FIELD_H

#include "../core/Types.h"
#include <string>
#include <vector>
#include <optional>

namespace ionet::schema {

/// Single bit flag within a bitfield
struct BitFlag {
    uint8_t bit;          // Bit position (0-63)
    std::string name;     // Flag name
    std::string description;
};

/// Scaling parameters for integer-to-real conversion
struct Scaling {
    double scale = 1.0;   // Multiplier
    double offset = 0.0;  // Added after scaling
    
    double apply(int64_t raw) const {
        return (static_cast<double>(raw) * scale) + offset;
    }
    
    int64_t remove(double real) const {
        return static_cast<int64_t>((real - offset) / scale);
    }
};

/// Validation constraints for a field
struct Constraints {
    std::optional<double> min;
    std::optional<double> max;
    std::optional<std::vector<int64_t>> validValues;  // Enum-like
};

/// Definition of a single field in a packet
struct Field {
    std::string name;
    core::DataType type = core::DataType::UInt8;
    
    // Size info
    std::optional<std::size_t> arraySize;    // For arrays
    std::optional<std::size_t> stringSize;   // For fixed-length strings
    std::optional<uint8_t> bitCount;         // For bitfields (1-64)
    
    // Interpretation
    std::optional<Scaling> scaling;
    std::optional<std::string> unit;
    std::string description;
    
    // Bitfield flags
    std::vector<BitFlag> bitFlags;
    
    // Validation
    Constraints constraints;
    
    /// Calculate byte size of this field
    std::size_t byteSize() const {
        if (arraySize) {
            return *arraySize * core::dataTypeSize(type);
        }
        if (stringSize) {
            return *stringSize;
        }
        if (bitCount) {
            return (*bitCount + 7) / 8;  // Round up to bytes
        }
        return core::dataTypeSize(type);
    }
    
    /// Check if field has fixed size
    bool isFixedSize() const {
        return type != core::DataType::String || stringSize.has_value();
    }
    
    /// Check if this is a bitfield
    bool isBitfield() const {
        return type == core::DataType::Bitfield || !bitFlags.empty();
    }
    
    /// Check if scaling should be applied
    bool hasScaling() const {
        return scaling.has_value();
    }
};

} // namespace ionet::schema

#endif