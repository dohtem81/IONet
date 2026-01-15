#ifndef IONET_SCHEMA_SOURCE_H
#define IONET_SCHEMA_SOURCE_H

#include "../core/Result.h"
#include "../core/Types.h"
#include <string>
#include <memory>

namespace ionet::schema {

/// Abstract source of schema content
class SchemaSource {
public:
    virtual ~SchemaSource() = default;
    
    /// Read the entire content as a string
    virtual core::Result<std::string> read() = 0;
    
    /// Get a description of this source (for error messages)
    virtual std::string description() const = 0;
};

/// Schema loaded from a file
class FileSource : public SchemaSource {
public:
    explicit FileSource(std::string path);
    
    core::Result<std::string> read() override;
    std::string description() const override;

private:
    std::string path_;
};

/// Schema from an in-memory string
class StringSource : public SchemaSource {
public:
    explicit StringSource(std::string content, std::string name = "string");
    
    core::Result<std::string> read() override;
    std::string description() const override;

private:
    std::string content_;
    std::string name_;
};

} // namespace ionet::schema

#endif