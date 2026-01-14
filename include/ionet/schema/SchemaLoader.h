#ifndef IONET_SCHEMA_LOADER_H
#define IONET_SCHEMA_LOADER_H

#include "Schema.h"
#include <filesystem>
#include <string_view>

namespace ionet::schema {

class SchemaLoader {
public:
    // Auto-detect format by extension
    static Schema loadFromFile(const std::filesystem::path& path);
    
    // Explicit format
    static Schema loadFromYaml(std::string_view content);
    static Schema loadFromJson(std::string_view content);
};

} // namespace ionet::schema

#endif