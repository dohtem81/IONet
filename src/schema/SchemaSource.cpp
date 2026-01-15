#include "../../include/ionet/schema/SchemaSource.h"
#include <fstream>
#include <sstream>

namespace ionet::schema {

// --- FileSource ---

FileSource::FileSource(std::string path) 
    : path_(std::move(path)) {}

core::Result<std::string> FileSource::read() {
    std::ifstream file(path_);
    
    if (!file.is_open()) {
        return core::Error{"Failed to open file: " + path_};
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    if (file.bad()) {
        return core::Error{"Failed to read file: " + path_};
    }
    
    return buffer.str();
}

std::string FileSource::description() const {
    return "file: " + path_;
}

// --- StringSource ---

StringSource::StringSource(std::string content, std::string name)
    : content_(std::move(content)), name_(std::move(name)) {}

core::Result<std::string> StringSource::read() {
    return content_;
}

std::string StringSource::description() const {
    return name_;
}

} // namespace ionet::schema