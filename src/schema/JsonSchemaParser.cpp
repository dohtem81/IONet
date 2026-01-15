#include "ionet/schema/SchemaParser.h"
#include <nlohmann/json.hpp>

namespace ionet::schema {

namespace {

using json = nlohmann::json;

ir::IRBitFlag parseIRBitFlag(const json& j) {
    ir::IRBitFlag flag;
    flag.bit = j["bit"].get<uint8_t>();
    flag.name = j["name"].get<std::string>();
    if (j.contains("description")) {
        flag.description = j["description"].get<std::string>();
    }
    return flag;
}

ir::IRField parseIRField(const json& j) {
    ir::IRField field;
    
    if (!j.contains("name")) {
        throw std::runtime_error("Field missing 'name'");
    }
    field.name = j["name"].get<std::string>();
    
    if (!j.contains("type")) {
        throw std::runtime_error("Field '" + field.name + "' missing 'type'");
    }
    field.type = j["type"].get<std::string>();
    
    if (j.contains("description")) {
        field.description = j["description"].get<std::string>();
    }
    if (j.contains("unit")) {
        field.unit = j["unit"].get<std::string>();
    }
    
    // Scaling
    if (j.contains("scale")) {
        field.scaling.scale = j["scale"].get<double>();
    }
    if (j.contains("offset")) {
        field.scaling.offset = j["offset"].get<double>();
    }
    
    // Constraints
    if (j.contains("min")) {
        field.constraints.min = j["min"].get<double>();
    }
    if (j.contains("max")) {
        field.constraints.max = j["max"].get<double>();
    }
    
    // Bitfield
    if (j.contains("bits")) {
        field.bitCount = j["bits"].get<uint8_t>();
    }
    if (j.contains("flags")) {
        for (const auto& flagJson : j["flags"]) {
            field.bitFlags.push_back(parseIRBitFlag(flagJson));
        }
    }
    
    // Size
    if (j.contains("size")) {
        field.size = j["size"].get<std::size_t>();
    }
    
    return field;
}

ir::IRPacket parseIRPacket(const json& j) {
    ir::IRPacket packet;
    
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
        packet.fields.push_back(parseIRField(fieldJson));
    }
    
    return packet;
}

} // anonymous namespace

ir::IRSchema JsonSchemaParser::parseToIR(std::string_view content) {
    json root = json::parse(content);
    
    ir::IRSchema ir;
    
    // Parse schema info
    if (root.contains("schema")) {
        const auto& schemaJson = root["schema"];
        if (schemaJson.contains("name")) {
            ir.info.name = schemaJson["name"].get<std::string>();
        }
        if (schemaJson.contains("version")) {
            ir.info.version = schemaJson["version"].get<std::string>();
        }
        if (schemaJson.contains("description")) {
            ir.info.description = schemaJson["description"].get<std::string>();
        }
        if (schemaJson.contains("byte_order")) {
            ir.info.byteOrder = schemaJson["byte_order"].get<std::string>();
        }
    }
    
    // Parse packets
    if (!root.contains("packets") || !root["packets"].is_array()) {
        throw std::runtime_error("Schema missing 'packets' array");
    }
    
    for (const auto& packetJson : root["packets"]) {
        ir.packets.push_back(parseIRPacket(packetJson));
    }
    
    return ir;
}

core::Result<Schema> JsonSchemaParser::parse(std::string_view content) {
    try {
        auto ir = parseToIR(content);
        return buildSchema(ir);
    } catch (const json::exception& e) {
        return core::Error{"JSON parse error: " + std::string(e.what())};
    } catch (const std::exception& e) {
        return core::Error{"Schema parse error: " + std::string(e.what())};
    }
}

} // namespace ionet::schema