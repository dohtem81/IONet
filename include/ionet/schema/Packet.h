#ifndef IONET_SCHEMA_PACKET_H
#define IONET_SCHEMA_PACKET_H

#include "Field.h"
#include <vector>
#include <string>
#include <optional>
#include <algorithm>

namespace ionet::schema {

/// Definition of a packet structure
struct Packet {
    uint32_t id = 0;                      // Packet identifier
    std::string name;
    std::string description;
    std::vector<Field> fields;
    
    /// Calculate total byte size of packet
    std::size_t totalSize() const {
        std::size_t size = 0;
        for (const auto& field : fields) {
            size += field.byteSize();
        }
        return size;
    }
    
    /// Check if all fields have fixed size
    bool isFixedSize() const {
        return std::all_of(fields.begin(), fields.end(),
            [](const Field& f) { return f.isFixedSize(); });
    }
    
    /// Find field by name
    const Field* findField(const std::string& name) const {
        auto it = std::find_if(fields.begin(), fields.end(),
            [&name](const Field& f) { return f.name == name; });
        return it != fields.end() ? &(*it) : nullptr;
    }
    
    /// Get field index by name, returns -1 if not found
    int fieldIndex(const std::string& name) const {
        for (std::size_t i = 0; i < fields.size(); ++i) {
            if (fields[i].name == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
};

} // namespace ionet::schema

#endif