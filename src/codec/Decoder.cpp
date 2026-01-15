#include "../../include/ionet/codec/Decoder.h"
#include <cmath>
#include <sstream>

namespace ionet::codec {

Decoder::Decoder(const schema::Schema& schema)
    : schema_(schema)
    , options_{}
{}

Decoder::Decoder(const schema::Schema& schema, DecodeOptions options)
    : schema_(schema)
    , options_(options)
{}

core::Result<DecodedPacket> Decoder::decode(
    uint32_t packetId,
    std::span<const uint8_t> data
) const {
    core::ByteBufferReader reader(data.data(), data.size());
    return decode(packetId, reader);
}

core::Result<DecodedPacket> Decoder::decode(
    uint32_t packetId,
    const std::vector<uint8_t>& data
) const {
    return decode(packetId, std::span<const uint8_t>(data));
}

core::Result<DecodedPacket> Decoder::decodeByName(
    const std::string& packetName,
    std::span<const uint8_t> data
) const {
    const auto* packet = schema_.findPacketByName(packetName);
    if (!packet) {
        return core::Error{"Unknown packet name: " + packetName};
    }
    return decode(packet->id, data);
}

core::Result<DecodedPacket> Decoder::decode(
    uint32_t packetId,
    core::ByteBufferReader& reader
) const {
    // Find packet definition
    const auto* packetDef = schema_.findPacketById(packetId);
    if (!packetDef) {
        return core::Error{"Unknown packet ID: " + std::to_string(packetId)};
    }
    
    DecodedPacket result(packetId, packetDef->name);
    core::ByteOrder byteOrder = schema_.byteOrder();
    
    // Decode each field
    for (const auto& fieldDef : packetDef->fields) {
        auto fieldResult = decodeField(fieldDef, reader, byteOrder);
        
        if (fieldResult.hasError()) {
            if (options_.stopOnError) {
                return core::Error{
                    "Failed to decode field '" + fieldDef.name + "': " +
                    fieldResult.error().message
                };
            }
            // Continue with next field if not stopping on error
            continue;
        }
        
        auto& decodedField = fieldResult.value();
        
        // Validate constraints
        if (options_.validateConstraints) {
            std::string constraintError;
            if (!validateConstraints(decodedField, fieldDef, &constraintError)) {
                if (options_.stopOnError) {
                    return core::Error{constraintError};
                }
            }
        }
        
        result.addField(std::move(decodedField));
    }
    
    return result;
}

core::Result<DecodedField> Decoder::decodeField(
    const schema::Field& fieldDef,
    core::ByteBufferReader& reader,
    core::ByteOrder byteOrder
) const {
    DecodedField field;
    field.name = fieldDef.name;
    field.type = fieldDef.type;
    field.unit = fieldDef.unit.value_or<std::string>("");
    
    try {
        switch (fieldDef.type) {
            case core::DataType::Int8: {
                auto val = reader.readInt8();
                field.rawValue = static_cast<int64_t>(val);
                break;
            }
            case core::DataType::Int16: {
                auto val = reader.readInt16(byteOrder);
                field.rawValue = static_cast<int64_t>(val);
                break;
            }
            case core::DataType::Int32: {
                auto val = reader.readInt32(byteOrder);
                field.rawValue = static_cast<int64_t>(val);
                break;
            }
            case core::DataType::Int64: {
                auto val = reader.readInt64(byteOrder);
                field.rawValue = static_cast<int64_t>(val);
                break;
            }
            case core::DataType::UInt8: {
                auto val = reader.readUInt8();
                field.rawValue = static_cast<uint64_t>(val);
                break;
            }
            case core::DataType::UInt16: {
                auto val = reader.readUInt16(byteOrder);
                field.rawValue = static_cast<uint64_t>(val);
                break;
            }
            case core::DataType::UInt32: {
                auto val = reader.readUInt32(byteOrder);
                field.rawValue = static_cast<uint64_t>(val);
                break;
            }
            case core::DataType::UInt64: {
                auto val = reader.readUInt64(byteOrder);
                field.rawValue = static_cast<uint64_t>(val);
                break;
            }
            case core::DataType::Float32: {
                auto val = reader.readFloat32(byteOrder);
                field.rawValue = static_cast<double>(val);
                break;
            }
            case core::DataType::Float64: {
                auto val = reader.readFloat64(byteOrder);
                field.rawValue = val;
                break;
            }
            case core::DataType::Bitfield: {
                uint64_t rawVal = 0;
                uint8_t bitCount = fieldDef.bitCount.value_or(8);
                
                if (bitCount <= 8) {
                    rawVal = reader.readUInt8();
                } else if (bitCount <= 16) {
                    rawVal = reader.readUInt16(byteOrder);
                } else if (bitCount <= 32) {
                    rawVal = reader.readUInt32(byteOrder);
                } else {
                    rawVal = reader.readUInt64(byteOrder);
                }
                
                field.rawValue = rawVal;
                field.bitfield = decodeBitfield(rawVal, fieldDef);
                break;
            }
            case core::DataType::String: {
                std::size_t size = fieldDef.stringSize.value_or(0);
                if (size == 0) {
                    return core::Error{"String field requires size"};
                }
                auto str = reader.readString(size);
                field.rawValue = str;
                break;
            }
            case core::DataType::Bytes: {
                std::size_t size = fieldDef.arraySize.value_or(0);
                if (size == 0) {
                    return core::Error{"Bytes field requires size"};
                }
                auto bytes = reader.readBytes(size);
                field.rawValue = bytes;
                break;
            }
            default:
                return core::Error{"Unsupported data type"};
        }
    } catch (const std::exception& e) {
        return core::Error{std::string("Read error: ") + e.what()};
    }
    
    // Apply scaling if enabled and defined
    if (options_.applyScaling && fieldDef.scaling.has_value()) {
        field.scaledValue = applyScaling(field.rawValue, *fieldDef.scaling);
    } else {
        field.scaledValue = field.rawValue;
    }
    
    return field;
}

DecodedBitfield Decoder::decodeBitfield(
    uint64_t rawValue,
    const schema::Field& fieldDef
) const {
    DecodedBitfield bf;
    bf.rawValue = rawValue;
    
    for (const auto& flag : fieldDef.bitFlags) {
        bool isSet = (rawValue >> flag.bit) & 1;
        bf.flags[flag.name] = isSet;
    }
    
    return bf;
}

core::Value Decoder::applyScaling(
    const core::Value& rawValue,
    const schema::Scaling& scaling
) const {
    double raw = 0.0;
    
    // Extract numeric value
    if (std::holds_alternative<int64_t>(rawValue)) {
        raw = static_cast<double>(std::get<int64_t>(rawValue));
    } else if (std::holds_alternative<uint64_t>(rawValue)) {
        raw = static_cast<double>(std::get<uint64_t>(rawValue));
    } else if (std::holds_alternative<double>(rawValue)) {
        raw = std::get<double>(rawValue);
    } else {
        // Non-numeric types don't get scaled
        return rawValue;
    }
    
    // Apply: scaled = (raw * scale) + offset
    double scaled = (raw * scaling.scale) + scaling.offset;
    return scaled;
}

bool Decoder::validateConstraints(
    const DecodedField& field,
    const schema::Field& fieldDef,
    std::string* errorMsg
) const {
    // Get the scaled value for validation
    double value = 0.0;
    bool hasValue = false;
    
    const auto& val = field.value();
    if (std::holds_alternative<int64_t>(val)) {
        value = static_cast<double>(std::get<int64_t>(val));
        hasValue = true;
    } else if (std::holds_alternative<uint64_t>(val)) {
        value = static_cast<double>(std::get<uint64_t>(val));
        hasValue = true;
    } else if (std::holds_alternative<double>(val)) {
        value = std::get<double>(val);
        hasValue = true;
    }
    
    if (!hasValue) {
        return true; // Non-numeric fields don't have constraints
    }
    
    // Check min constraint
    if (fieldDef.constraints.min.has_value()) {
        if (value < *fieldDef.constraints.min) {
            if (errorMsg) {
                std::ostringstream oss;
                oss << "Field '" << field.name << "' value " << value
                    << " is below minimum " << *fieldDef.constraints.min;
                *errorMsg = oss.str();
            }
            return false;
        }
    }
    
    // Check max constraint
    if (fieldDef.constraints.max.has_value()) {
        if (value > *fieldDef.constraints.max) {
            if (errorMsg) {
                std::ostringstream oss;
                oss << "Field '" << field.name << "' value " << value
                    << " is above maximum " << *fieldDef.constraints.max;
                *errorMsg = oss.str();
            }
            return false;
        }
    }
    
    return true;
}

} // namespace ionet::codec
