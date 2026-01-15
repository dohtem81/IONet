#include "ionet/schema/SchemaLoader.h"
#include <algorithm>
#include <cctype>

namespace ionet::schema {

SchemaFormat SchemaLoader::detectFormat(std::string_view content) {
    // Skip whitespace
    auto it = std::find_if(content.begin(), content.end(), 
        [](char c) { return !std::isspace(static_cast<unsigned char>(c)); });
    
    if (it == content.end()) {
        return SchemaFormat::YAML; // Default to YAML for empty content
    }
    
    // JSON starts with { or [
    if (*it == '{' || *it == '[') {
        return SchemaFormat::JSON;
    }
    
    // Default to YAML
    return SchemaFormat::YAML;
}

std::unique_ptr<SchemaParser> SchemaLoader::createParser(SchemaFormat format) {
    switch (format) {
        case SchemaFormat::YAML:
            return std::make_unique<YamlSchemaParser>();
        case SchemaFormat::JSON:
            return std::make_unique<JsonSchemaParser>();
        case SchemaFormat::Auto:
            // Should not happen, detect first
            return std::make_unique<YamlSchemaParser>();
    }
    return std::make_unique<YamlSchemaParser>();
}

core::Result<Schema> SchemaLoader::load(
    std::unique_ptr<SchemaSource> source,
    SchemaFormat format
) {
    auto contentResult = source->read();
    if (contentResult.hasError()) {
        return core::Error{
            "Failed to read from " + source->description() + ": " + 
            contentResult.error().message
        };
    }
    
    const auto& content = contentResult.value();
    
    if (format == SchemaFormat::Auto) {
        format = detectFormat(content);
    }
    
    auto parser = createParser(format);
    auto result = parser->parse(content);
    
    if (result.hasError()) {
        return core::Error{
            "Failed to parse " + source->description() + ": " +
            result.error().message
        };
    }
    
    return result;
}

core::Result<Schema> SchemaLoader::fromFile(
    const std::string& path,
    SchemaFormat format
) {
    return load(std::make_unique<FileSource>(path), format);
}

core::Result<Schema> SchemaLoader::fromString(
    std::string_view content,
    SchemaFormat format
) {
    return load(std::make_unique<StringSource>(std::string(content)), format);
}

core::Result<Schema> SchemaLoader::fromYaml(std::string_view content) {
    return fromString(content, SchemaFormat::YAML);
}

core::Result<Schema> SchemaLoader::fromJson(std::string_view content) {
    return fromString(content, SchemaFormat::JSON);
}

} // namespace ionet::schema