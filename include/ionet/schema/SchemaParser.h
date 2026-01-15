#ifndef IONET_SCHEMA_PARSER_H
#define IONET_SCHEMA_PARSER_H

#include "Schema.h"
#include "../core/Result.h"
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <vector>

namespace ionet::schema {

/// Format detection hint
enum class SchemaFormat {
    Auto,   // Detect from content
    YAML,
    JSON
};

/// Abstract parser interface
class SchemaParser {
public:
    virtual ~SchemaParser() = default;
    
    /// Parse schema content into a Schema object
    virtual core::Result<Schema> parse(std::string_view content) = 0;
    
    /// Get the format this parser handles
    virtual SchemaFormat format() const = 0;
};

/// Intermediate representation for parsing
/// This allows format-specific parsers to convert to common structure
namespace ir {

struct IRBitFlag {
    uint8_t bit = 0;
    std::string name;
    std::string description;
};

struct IRScaling {
    std::optional<double> scale;
    std::optional<double> offset;
};

struct IRConstraints {
    std::optional<double> min;
    std::optional<double> max;
};

struct IRField {
    std::string name;
    std::string type;
    std::string description;
    std::string unit;
    IRScaling scaling;
    IRConstraints constraints;
    std::optional<uint8_t> bitCount;
    std::vector<IRBitFlag> bitFlags;
    std::optional<std::size_t> size;
};

struct IRPacket {
    uint32_t id = 0;
    std::string name;
    std::string description;
    std::vector<IRField> fields;
};

struct IRSchemaInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string byteOrder;
};

struct IRSchema {
    IRSchemaInfo info;
    std::vector<IRPacket> packets;
};

} // namespace ir

/// Base class with common schema building logic
class SchemaParserBase : public SchemaParser {
protected:
    /// Convert intermediate representation to Schema
    core::Result<Schema> buildSchema(const ir::IRSchema& ir);
    
    /// Parse data type string to enum
    core::DataType parseDataType(const std::string& typeStr);
    
    /// Parse byte order string to enum
    core::ByteOrder parseByteOrder(const std::string& orderStr);
    
    /// Build Field from IR
    Field buildField(const ir::IRField& irField);
    
    /// Build Packet from IR
    Packet buildPacket(const ir::IRPacket& irPacket);
};

/// YAML schema parser
class YamlSchemaParser : public SchemaParserBase {
public:
    core::Result<Schema> parse(std::string_view content) override;
    SchemaFormat format() const override { return SchemaFormat::YAML; }

private:
    ir::IRSchema parseToIR(std::string_view content);
};

/// JSON schema parser
class JsonSchemaParser : public SchemaParserBase {
public:
    core::Result<Schema> parse(std::string_view content) override;
    SchemaFormat format() const override { return SchemaFormat::JSON; }

private:
    ir::IRSchema parseToIR(std::string_view content);
};

} // namespace ionet::schema

#endif