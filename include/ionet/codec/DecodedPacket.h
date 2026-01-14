#ifndef IONET_CODEC_DECODED_PACKET_H
#define IONET_CODEC_DECODED_PACKET_H

#include "../core/Types.h"
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ionet::codec {

/// Decoded bitfield represented as name->value map
using BitfieldValue = std::unordered_map<std::string, bool>;

/// Decoded array
using ArrayValue = std::vector<core::Value>;

/// A single decoded field value
using FieldValue = std::variant<
    std::monostate,     // Empty/null
    int64_t,            // Signed integers
    uint64_t,           // Unsigned integers  
    double,             // Floats (scaled or raw)
    std::string,        // Strings
    BitfieldValue,      // Bitfield flags
    ArrayValue          // Arrays
>;

/// Result of decoding a packet
struct DecodedPacket {
    uint32_t packetId = 0;
    std::string packetName;
    std::unordered_map<std::string, FieldValue> fields;
    
    /// Get field value by name (throws if not found)
    const FieldValue& operator[](const std::string& name) const {
        return fields.at(name);
    }
    
    /// Check if field exists
    bool hasField(const std::string& name) const {
        return fields.find(name) != fields.end();
    }
    
    /// Get field count
    std::size_t fieldCount() const {
        return fields.size();
    }
};

} // namespace ionet::codec

#endif
