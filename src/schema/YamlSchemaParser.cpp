#include "../../include/ionet/schema/SchemaParser.h"
#include <yaml-cpp/yaml.h>

namespace ionet::schema {

namespace {

ir::IRBitFlag parseIRBitFlag(const YAML::Node& node) {
    ir::IRBitFlag flag;
    flag.bit = node["bit"].as<uint8_t>();
    flag.name = node["name"].as<std::string>();
    if (node["description"]) {
        flag.description = node["description"].as<std::string>();
    }
    return flag;
}

ir::IRField parseIRField(const YAML::Node& node) {
    ir::IRField field;
    
    if (!node["name"]) {
        throw std::runtime_error("Field missing 'name'");
    }
    field.name = node["name"].as<std::string>();
    
    if (!node["type"]) {
        throw std::runtime_error("Field '" + field.name + "' missing 'type'");
    }
    field.type = node["type"].as<std::string>();
    
    if (node["description"]) {
        field.description = node["description"].as<std::string>();
    }
    if (node["unit"]) {
        field.unit = node["unit"].as<std::string>();
    }
    
    // Scaling
    if (node["scale"]) {
        field.scaling.scale = node["scale"].as<double>();
    }
    if (node["offset"]) {
        field.scaling.offset = node["offset"].as<double>();
    }
    
    // Constraints
    if (node["min"]) {
        field.constraints.min = node["min"].as<double>();
    }
    if (node["max"]) {
        field.constraints.max = node["max"].as<double>();
    }
    
    // Bitfield
    if (node["bits"]) {
        field.bitCount = node["bits"].as<uint8_t>();
    }
    if (node["flags"]) {
        for (const auto& flagNode : node["flags"]) {
            field.bitFlags.push_back(parseIRBitFlag(flagNode));
        }
    }
    
    // Size
    if (node["size"]) {
        field.size = node["size"].as<std::size_t>();
    }
    
    return field;
}

ir::IRPacket parseIRPacket(const YAML::Node& node) {
    ir::IRPacket packet;
    
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
        packet.fields.push_back(parseIRField(fieldNode));
    }
    
    return packet;
}

} // anonymous namespace

ir::IRSchema YamlSchemaParser::parseToIR(std::string_view content) {
    YAML::Node root = YAML::Load(std::string(content));
    
    ir::IRSchema ir;
    
    // Parse schema info
    if (root["schema"]) {
        const auto& schemaNode = root["schema"];
        if (schemaNode["name"]) {
            ir.info.name = schemaNode["name"].as<std::string>();
        }
        if (schemaNode["version"]) {
            ir.info.version = schemaNode["version"].as<std::string>();
        }
        if (schemaNode["description"]) {
            ir.info.description = schemaNode["description"].as<std::string>();
        }
        if (schemaNode["byte_order"]) {
            ir.info.byteOrder = schemaNode["byte_order"].as<std::string>();
        }
    }
    
    // Parse packets
    if (!root["packets"] || !root["packets"].IsSequence()) {
        throw std::runtime_error("Schema missing 'packets' array");
    }
    
    for (const auto& packetNode : root["packets"]) {
        ir.packets.push_back(parseIRPacket(packetNode));
    }
    
    return ir;
}

core::Result<Schema> YamlSchemaParser::parse(std::string_view content) {
    try {
        auto ir = parseToIR(content);
        return buildSchema(ir);
    } catch (const YAML::Exception& e) {
        return core::Error{"YAML parse error: " + std::string(e.what())};
    } catch (const std::exception& e) {
        return core::Error{"Schema parse error: " + std::string(e.what())};
    }
}

} // namespace ionet::schema