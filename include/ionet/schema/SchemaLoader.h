#ifndef IONET_SCHEMA_LOADER_H
#define IONET_SCHEMA_LOADER_H

#include "Schema.h"
#include "SchemaSource.h"
#include "SchemaParser.h"
#include "../core/Result.h"
#include <memory>
#include <string>
#include <string_view>

namespace ionet::schema {

/// Main entry point for loading schemas
class SchemaLoader {
public:
    /// Load from any source with format auto-detection
    static core::Result<Schema> load(
        std::unique_ptr<SchemaSource> source,
        SchemaFormat format = SchemaFormat::Auto
    );
    
    /// Convenience: load from file path
    static core::Result<Schema> fromFile(
        const std::string& path,
        SchemaFormat format = SchemaFormat::Auto
    );
    
    /// Convenience: load from string
    static core::Result<Schema> fromString(
        std::string_view content,
        SchemaFormat format = SchemaFormat::Auto
    );
    
    /// Convenience: load from YAML string
    static core::Result<Schema> fromYaml(std::string_view content);
    
    /// Convenience: load from JSON string
    static core::Result<Schema> fromJson(std::string_view content);

private:
    static SchemaFormat detectFormat(std::string_view content);
    static std::unique_ptr<SchemaParser> createParser(SchemaFormat format);
};

} // namespace ionet::schema

#endif