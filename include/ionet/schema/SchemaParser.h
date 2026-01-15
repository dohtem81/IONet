#ifndef IONET_SCHEMA_PARSER_H
#define IONET_SCHEMA_PARSER_H

#include "Schema.h"
#include "../core/Result.h"
#include <string>
#include <string_view>
#include <memory>

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

/// YAML schema parser
class YamlSchemaParser : public SchemaParser {
public:
    core::Result<Schema> parse(std::string_view content) override;
    SchemaFormat format() const override { return SchemaFormat::YAML; }
};

/// JSON schema parser
class JsonSchemaParser : public SchemaParser {
public:
    core::Result<Schema> parse(std::string_view content) override;
    SchemaFormat format() const override { return SchemaFormat::JSON; }
};

} // namespace ionet::schema

#endif