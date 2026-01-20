#include <iostream>
#include <../../include/ionet/schema/SchemaLoader.h>

int main() {
    // Load heartbeat schema definition
    std::string schema_path = "./config/schema/heartbeat_processes.yaml";
    auto schema_result = ionet::schema::SchemaLoader::fromFile(schema_path);
    if (!schema_result.ok()) {
        std::cerr << "Failed to load schema: " << schema_result.error().message << std::endl;
        return 1;
    }
    const auto& schema = schema_result.value();
    std::cout << "Loaded schema: " << schema.info().name << " v" << schema.info().version << std::endl;
    // ... further logic will go here ...
    return 0;
}
