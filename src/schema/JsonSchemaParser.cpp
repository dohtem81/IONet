#include "../../include/ionet/schema/SchemaParser.h"
#include "../../include/ionet/core/Types.h"
#include <nlohmann/json.hpp>

namespace ionet::schema {

namespace {

using json = nlohmann::json;

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

Field parseField(const json& j) {
    Field field;
    
    if (!j.contains("name")) {
        throw std::runtime_error("Field missing 'name'");
    }
    field.name = j["name"].get<std::string>();
    
    if (!j.contains("type")) {
        throw std::runtime_error("Field '" + field.name + "' missing 'type'");
    }
    field.type = parseDataType(j["type"].get<std::string>());
    
    if (j.contains("description")) {
        field.description = j["description"].get<std::string>();
    }
    
    if (j.contains("unit")) {
        field.unit = j["unit"].get<std::string>();
    }
    
    if (j.contains("scale") || j.contains("offset")) {
        Scaling scaling;
        if (j.contains("scale")) {
            scaling.scale = j["scale"].get<double>();
        }
        if (j.contains("offset")) {
            scaling.offset = j["offset"].get<double>();
        }
        field.scaling = scaling;
    }
    
    if (j.contains("bits")) {
        field.bitCount = j["bits"].get<uint8_t>();
    }
    
    if (j.contains("flags")) {
        for (const auto& flagJson : j["flags"]) {
            BitFlag flag;
            flag.bit = flagJson["bit"].get<uint8_t>();
            flag.name = flagJson["name"].get<std::string>();
            if (flagJson.contains("description")) {
                flag.description = flagJson["description"].get<std::string>();
            }
            field.bitFlags.push_back(std::move(flag));
        }
    }
    
    if (j.contains("size")) {
        if (field.type == core::DataType::String) {
            field.stringSize = j["size"].get<std::size_t>();
        } else {
            field.arraySize = j["size"].get<std::size_t>();
        }
    }
    
    if (j.contains("min")) {
        field.constraints.min = j["min"].get<double>();
    }
    if (j.contains("max")) {
        field.constraints.max = j["max"].get<double>();
    }
    
    return field;
}

Packet parsePacket(const json& j) {
    Packet packet;
    
    if (!j.contains("id")) {
        throw std::runtime_error("Packet missing 'id'");
    }
    packet.id = j["id"].get<uint32_t>();
    
    if (!j.contains("name")) {
        throw std::runtime_error("Packet missing 'name'");
    }
    packet.name = j["name"].get<std::string>();
    
    if (j.contains("description")) {
        packet.description = j["description"].get<std::string>();
    }
    
    if (!j.contains("fields") || !j["fields"].is_array()) {
        throw std::runtime_error("Packet '" + packet.name + "' missing 'fields'");
    }
    
    for (const auto& fieldJson : j["fields"]) {
        packet.fields.push_back(parseField(fieldJson));
    }
    
    return packet;
}

} // anonymous namespace

core::Result<Schema> JsonSchemaParser::parse(std::string_view content) {
    try {
        json root = json::parse(content);
        
        Schema schema;
        
        if (root.contains("schema")) {
            const auto& schemaJson = root["schema"];
            SchemaInfo info;
            
            if (schemaJson.contains("name")) {
                info.name = schemaJson["name"].get<std::string>();
            }
            if (schemaJson.contains("version")) {
                info.version = schemaJson["version"].get<std::string>();
            }
            if (schemaJson.contains("description")) {
                info.description = schemaJson["description"].get<std::string>();
            }
            
            schema.setInfo(std::move(info));
            
            if (schemaJson.contains("byte_order")) {
                schema.setByteOrder(parseByteOrder(schemaJson["byte_order"].get<std::string>()));
            }
        }
        
        if (!root.contains("packets") || !root["packets"].is_array()) {
            return core::Error{"Schema missing 'packets' array"};
        }
        
        for (const auto& packetJson : root["packets"]) {
            schema.addPacket(parsePacket(packetJson));
        }
        
        std::string validationError;
        if (!schema.validate(&validationError)) {
            return core::Error{"Schema validation failed: " + validationError};
        }
        
        return schema;
        
    } catch (const json::exception& e) {
        return core::Error{"JSON parse error: " + std::string(e.what())};
    } catch (const std::exception& e) {
        return core::Error{"Schema parse error: " + std::string(e.what())};
    }
}

} // namespace ionet::schema