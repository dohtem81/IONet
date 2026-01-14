#include "ionet/codec/Decoder.h"
#include <stdexcept>

namespace ionet::codec {

DecodeResult Decoder::decode(uint32_t packetId, std::span<const uint8_t> data) const {
    // Find packet definition
    const auto* packet = schema_.findPacketById(packetId);
    if (!packet) {
        return DecodeError{
            "Unknown packet ID: 0x" + std::to_string(packetId),
            0,
            ""
        };
    }
    
    // Check minimum size for fixed-size packets
    if (packet->isFixedSize() && data.size() < packet->totalSize()) {
        return DecodeError{
            "Insufficient data: expected " + std::to_string(packet->totalSize()) +
            " bytes, got " + std::to_string(data.size()),
            0,
            ""
        };
    }
    
    // Create buffer with schema's byte order
    core::ByteBuffer buffer(data, schema_.byteOrder());
    
    DecodedPacket result;
    result.packetId = packetId;
    result.packetName = packet->name;
    
    // Decode each field
    for (const auto& field : packet->fields) {
        try {
            auto value = decodeField(field, buffer);
            
            // Validate if constraints are present
            if (!validateField(value, field)) {
                return DecodeError{
                    "Field validation failed for '" + field.name + "'",
                    buffer.position(),
                    field.name
                };
            }
            
            result.fields[field.name] = std::move(value);
            
        } catch (const std::out_of_range& e) {
            return DecodeError{
                "Buffer underflow while reading field '" + field.name + "': " + e.what(),
                buffer.position(),
                field.name
            };
        } catch (const std::exception& e) {
            return DecodeError{
                "Error decoding field '" + field.name + "': " + e.what(),
                buffer.position(),
                field.name
            };
        }
    }
    
    return result;
}

DecodeResult Decoder::decodeByName(const std::string& packetName, std::span<const uint8_t> data) const {
    const auto* packet = schema_.findPacketByName(packetName);
    if (!packet) {
        return DecodeError{
            "Unknown packet name: " + packetName,
            0,
            ""
        };
    }
    return decode(packet->id, data);
}

FieldValue Decoder::decodeField(const schema::Field& field, core::ByteBuffer& buffer) const {
    using namespace core;
    
    // Handle arrays
    if (field.arraySize) {
        ArrayValue array;
        array.reserve(*field.arraySize);
        
        for (std::size_t i = 0; i < *field.arraySize; ++i) {
            // Recursively decode array elements (simplified - create temp field without array)
            schema::Field elementField = field;
            elementField.arraySize.reset();
            
            auto elementValue = decodeField(elementField, buffer);
            
            // Convert FieldValue to core::Value
            if (std::holds_alternative<int64_t>(elementValue)) {
                array.push_back(std::get<int64_t>(elementValue));
            } else if (std::holds_alternative<uint64_t>(elementValue)) {
                array.push_back(std::get<uint64_t>(elementValue));
            } else if (std::holds_alternative<double>(elementValue)) {
                array.push_back(std::get<double>(elementValue));
            } else if (std::holds_alternative<std::string>(elementValue)) {
                array.push_back(std::get<std::string>(elementValue));
            }
        }
        
        return array;
    }
    
    // Handle strings
    if (field.type == DataType::String) {
        if (field.stringSize) {
            auto bytes = buffer.readBytes(*field.stringSize);
            // Find null terminator or use full size
            std::size_t len = 0;
            while (len < bytes.size() && bytes[len] != 0) {
                ++len;
            }
            return std::string(reinterpret_cast<const char*>(bytes.data()), len);
        } else {
            throw std::runtime_error("Variable-length strings not yet supported");
        }
    }
    
    // Handle bitfields
    if (field.isBitfield()) {
        uint64_t rawBits = decodeBitfield(field, buffer);
        if (!field.bitFlags.empty()) {
            return extractBitFlags(rawBits, field.bitFlags);
        } else {
            return rawBits;
        }
    }
    
    // Handle numeric types
    switch (field.type) {
        case DataType::Int8: {
            int8_t val = buffer.read<int8_t>();
            if (field.hasScaling()) {
                return field.scaling->apply(val);
            }
            return static_cast<int64_t>(val);
        }
        case DataType::Int16: {
            int16_t val = buffer.read<int16_t>();
            if (field.hasScaling()) {
                return field.scaling->apply(val);
            }
            return static_cast<int64_t>(val);
        }
        case DataType::Int32: {
            int32_t val = buffer.read<int32_t>();
            if (field.hasScaling()) {
                return field.scaling->apply(val);
            }
            return static_cast<int64_t>(val);
        }
        case DataType::Int64: {
            int64_t val = buffer.read<int64_t>();
            if (field.hasScaling()) {
                return field.scaling->apply(val);
            }
            return val;
        }
        case DataType::UInt8: {
            uint8_t val = buffer.read<uint8_t>();
            if (field.hasScaling()) {
                return field.scaling->apply(static_cast<int64_t>(val));
            }
            return static_cast<uint64_t>(val);
        }
        case DataType::UInt16: {
            uint16_t val = buffer.read<uint16_t>();
            if (field.hasScaling()) {
                return field.scaling->apply(static_cast<int64_t>(val));
            }
            return static_cast<uint64_t>(val);
        }
        case DataType::UInt32: {
            uint32_t val = buffer.read<uint32_t>();
            if (field.hasScaling()) {
                return field.scaling->apply(static_cast<int64_t>(val));
            }
            return static_cast<uint64_t>(val);
        }
        case DataType::UInt64: {
            uint64_t val = buffer.read<uint64_t>();
            if (field.hasScaling()) {
                return field.scaling->apply(static_cast<int64_t>(val));
            }
            return val;
        }
        case DataType::Float32: {
            float val = buffer.read<float>();
            return static_cast<double>(val);
        }
        case DataType::Float64: {
            return buffer.read<double>();
        }
        default:
            throw std::runtime_error("Unsupported data type");
    }
}

uint64_t Decoder::decodeBitfield(const schema::Field& field, core::ByteBuffer& buffer) const {
    if (!field.bitCount) {
        throw std::runtime_error("Bitfield must specify bit count");
    }
    
    return buffer.readBits(*field.bitCount);
}

BitfieldValue Decoder::extractBitFlags(uint64_t raw, const std::vector<schema::BitFlag>& flags) const {
    BitfieldValue result;
    
    for (const auto& flag : flags) {
        bool isSet = (raw >> flag.bit) & 0x01;
        result[flag.name] = isSet;
    }
    
    return result;
}

bool Decoder::validateField(const FieldValue& value, const schema::Field& field) const {
    const auto& constraints = field.constraints;
    
    // No constraints means always valid
    if (!constraints.min && !constraints.max && !constraints.validValues) {
        return true;
    }
    
    // Extract numeric value for validation
    double numericValue = 0.0;
    bool isNumeric = false;
    
    if (std::holds_alternative<int64_t>(value)) {
        numericValue = static_cast<double>(std::get<int64_t>(value));
        isNumeric = true;
    } else if (std::holds_alternative<uint64_t>(value)) {
        numericValue = static_cast<double>(std::get<uint64_t>(value));
        isNumeric = true;
    } else if (std::holds_alternative<double>(value)) {
        numericValue = std::get<double>(value);
        isNumeric = true;
    }
    
    if (isNumeric) {
        // Check min/max
        if (constraints.min && numericValue < *constraints.min) {
            return false;
        }
        if (constraints.max && numericValue > *constraints.max) {
            return false;
        }
        
        // Check valid values (for enums)
        if (constraints.validValues) {
            int64_t intValue = static_cast<int64_t>(numericValue);
            bool found = false;
            for (int64_t valid : *constraints.validValues) {
                if (intValue == valid) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
    }
    
    return true;
}

} // namespace ionet::codec
