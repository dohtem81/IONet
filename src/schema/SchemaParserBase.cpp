#include "ionet/schema/SchemaParser.h"
#include <stdexcept>

namespace ionet::schema {

core::DataType SchemaParserBase::parseDataType(const std::string& typeStr) {
    if (typeStr == "int8")     return core::DataType::Int8;
    if (typeStr == "int16")    return core::DataType::Int16;
    if (typeStr == "int32")    return core::DataType::Int32;
    if (typeStr == "int64")    return core::DataType::Int64;
    if (typeStr == "uint8")    return core::DataType::UInt8;
    if (typeStr == "uint16")   return core::DataType::UInt16;
    if (typeStr == "uint32")   return core::DataType::UInt32;
    if (typeStr == "uint64")   return core::DataType::UInt64;
    if (typeStr == "float32")  return core::DataType::Float32;
    if (typeStr == "float64")  return core::DataType::Float64;
    if (typeStr == "bitfield") return core::DataType::Bitfield;
    if (typeStr == "string")   return core::DataType::String;
    if (typeStr == "bytes")    return core::DataType::Bytes;
    
    throw std::runtime_error("Unknown data type: " + typeStr);
}

core::ByteOrder SchemaParserBase::parseByteOrder(const std::string& orderStr) {
    if (orderStr.empty()) {
        return core::ByteOrder::Native;
    }
    if (orderStr == "big" || orderStr == "be" || orderStr == "big_endian") {
        return core::ByteOrder::Big;
    }
    if (orderStr == "little" || orderStr == "le" || orderStr == "little_endian") {
        return core::ByteOrder::Little;
    }
    if (orderStr == "native") {
        return core::ByteOrder::Native;
    }
    
    throw std::runtime_error("Unknown byte order: " + orderStr);
}

Field SchemaParserBase::buildField(const ir::IRField& irField) {
    Field field;
    
    field.name = irField.name;
    field.type = parseDataType(irField.type);
    field.description = irField.description;
    field.unit = irField.unit;
    
    // Scaling
    if (irField.scaling.scale.has_value() || irField.scaling.offset.has_value()) {
        Scaling scaling;
        scaling.scale = irField.scaling.scale.value_or(1.0);
        scaling.offset = irField.scaling.offset.value_or(0.0);
        field.scaling = scaling;
    }
    
    // Constraints
    field.constraints.min = irField.constraints.min;
    field.constraints.max = irField.constraints.max;
    
    // Bitfield
    if (irField.bitCount.has_value()) {
        field.bitCount = irField.bitCount.value();
    }
    
    for (const auto& irFlag : irField.bitFlags) {
        BitFlag flag;
        flag.bit = irFlag.bit;
        flag.name = irFlag.name;
        flag.description = irFlag.description;
        field.bitFlags.push_back(std::move(flag));
    }
    
    // Size (for strings/arrays)
    if (irField.size.has_value()) {
        if (field.type == core::DataType::String) {
            field.stringSize = irField.size.value();
        } else {
            field.arraySize = irField.size.value();
        }
    }
    
    return field;
}

Packet SchemaParserBase::buildPacket(const ir::IRPacket& irPacket) {
    Packet packet;
    
    packet.id = irPacket.id;
    packet.name = irPacket.name;
    packet.description = irPacket.description;
    
    for (const auto& irField : irPacket.fields) {
        packet.fields.push_back(buildField(irField));
    }
    
    return packet;
}

core::Result<Schema> SchemaParserBase::buildSchema(const ir::IRSchema& ir) {
    try {
        Schema schema;
        
        // Schema info
        SchemaInfo info;
        info.name = ir.info.name;
        info.version = ir.info.version;
        info.description = ir.info.description;
        schema.setInfo(std::move(info));
        
        // Byte order
        if (!ir.info.byteOrder.empty()) {
            schema.setByteOrder(parseByteOrder(ir.info.byteOrder));
        }
        
        // Packets
        for (const auto& irPacket : ir.packets) {
            schema.addPacket(buildPacket(irPacket));
        }
        
        // Validate
        std::string validationError;
        if (!schema.validate(&validationError)) {
            return core::Error{"Schema validation failed: " + validationError};
        }
        
        return schema;
        
    } catch (const std::exception& e) {
        return core::Error{"Schema build error: " + std::string(e.what())};
    }
}

} // namespace ionet::schema