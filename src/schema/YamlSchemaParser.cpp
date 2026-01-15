#include "../../include/ionet/schema/SchemaParser.h"
#include "../../include/ionet/core/Types.h"
#include <yaml-cpp/yaml.h>
#include <stdexcept>

namespace ionet::schema {

namespace {

core::DataType parseDataType(const std::string& typeStr) {
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
    
    throw std::runtime_error("Unknown data type: " + typeStr);
}

core::ByteOrder parseByteOrder(const std::string& orderStr) {
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

Field parseField(const YAML::Node& node) {
    Field field;
    
    if (!node["name"]) {
        throw std::runtime_error("Field missing 'name'");
    }
    field.name = node["name"].as<std::string>();
    
    if (!node["type"]) {
        throw std::runtime_error("Field '" + field.name + "' missing 'type'");
    }
    field.type = parseDataType(node["type"].as<std::string>());
    
    // Optional properties
    if (node["description"]) {
        field.description = node["description"].as<std::string>();
    }
    
    if (node["unit"]) {
        field.unit = node["unit"].as<std::string>();
    }
    
    // Scaling
    if (node["scale"] || node["offset"]) {
        Scaling scaling;
        if (node["scale"]) {
            scaling.scale = node["scale"].as<double>();
        }
        if (node["offset"]) {
            scaling.offset = node["offset"].as<double>();
        }
        field.scaling = scaling;
    }
    
    // Bitfield specific
    if (node["bits"]) {
        field.bitCount = node["bits"].as<uint8_t>();
    }
    
    if (node["flags"]) {
        for (const auto& flagNode : node["flags"]) {
            BitFlag flag;
            flag.bit = flagNode["bit"].as<uint8_t>();
            flag.name = flagNode["name"].as<std::string>();
            if (flagNode["description"]) {
                flag.description = flagNode["description"].as<std::string>();
            }
            field.bitFlags.push_back(std::move(flag));
        }
    }
    
    // Array/string size
    if (node["size"]) {
        if (field.type == core::DataType::String) {
            field.stringSize = node["size"].as<std::size_t>();
        } else {
            field.arraySize = node["size"].as<std::size_t>();
        }
    }
    
    // Constraints
    if (node["min"]) {
        field.constraints.min = node["min"].as<double>();
    }
    if (node["max"]) {
        field.constraints.max = node["max"].as<double>();
    }
    
    return field;
}

Packet parsePacket(const YAML::Node& node) {
    Packet packet;
    
    if (!node["id"]) {
        throw std::runtime_error("Packet missing 'id'");
    }
    packet.id = node["id"].as<uint32_t>();
    
    if (!node["name"]) {
        throw std::runtime_error("Packet missing 'name'");
    }
    packet.name = node["name"].as<std::string>();
    
    if (node["description"]) {
        packet.description = node["description"].as<std::string>();
    }
    
    if (!node["fields"] || !node["fields"].IsSequence()) {
        throw std::runtime_error("Packet '" + packet.name + "' missing 'fields'");
    }
    
    for (const auto& fieldNode : node["fields"]) {
        packet.fields.push_back(parseField(fieldNode));
    }
    
    return packet;
}

} // anonymous namespace

core::Result<Schema> YamlSchemaParser::parse(std::string_view content) {
    try {
        YAML::Node root = YAML::Load(std::string(content));
        
        Schema schema;
        
        // Parse schema info
        if (root["schema"]) {
            const auto& schemaNode = root["schema"];
            SchemaInfo info;
            
            if (schemaNode["name"]) {
                info.name = schemaNode["name"].as<std::string>();
            }
            if (schemaNode["version"]) {
                info.version = schemaNode["version"].as<std::string>();
            }
            if (schemaNode["description"]) {
                info.description = schemaNode["description"].as<std::string>();
            }
            
            schema.setInfo(std::move(info));
            
            if (schemaNode["byte_order"]) {
                schema.setByteOrder(parseByteOrder(schemaNode["byte_order"].as<std::string>()));
            }
        }
        
        // Parse packets
        if (!root["packets"] || !root["packets"].IsSequence()) {
            return core::Error{"Schema missing 'packets' array"};
        }
        
        for (const auto& packetNode : root["packets"]) {
            schema.addPacket(parsePacket(packetNode));
        }
        
        // Validate
        std::string validationError;
        if (!schema.validate(&validationError)) {
            return core::Error{"Schema validation failed: " + validationError};
        }
        
        return schema;
        
    } catch (const YAML::Exception& e) {
        return core::Error{"YAML parse error: " + std::string(e.what())};
    } catch (const std::exception& e) {
        return core::Error{"Schema parse error: " + std::string(e.what())};
    }
}

} // namespace ionet::schema