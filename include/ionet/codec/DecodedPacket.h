#ifndef IONET_CODEC_DECODED_PACKET_H
#define IONET_CODEC_DECODED_PACKET_H

#include "../core/Types.h"
#include "../schema/Packet.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <cstdint>

namespace ionet::codec {

/// Decoded bitfield with named flags
struct DecodedBitfield {
    uint64_t rawValue = 0;
    std::unordered_map<std::string, bool> flags;
    
    /// Check if a specific flag is set
    bool isSet(const std::string& flagName) const;
    
    /// Check if a specific bit is set
    bool bitAt(uint8_t bit) const;
};

/// A single decoded field value
struct DecodedField {
    std::string name;
    core::DataType type;
    core::Value rawValue;           // Value before scaling
    core::Value scaledValue;        // Value after scaling (if applicable)
    std::string unit;
    
    /// For bitfields
    std::optional<DecodedBitfield> bitfield;
    
    /// Get value as specific type (scaled if available, raw otherwise)
    template<typename T>
    std::optional<T> as() const;
    
    /// Get the display value (scaled if available)
    core::Value value() const;
    
    /// Check if field has scaling applied
    bool hasScaling() const;
};

/// Container for a fully decoded packet
class DecodedPacket {
public:
    DecodedPacket() = default;
    DecodedPacket(uint32_t packetId, std::string packetName);
    
    /// Packet identification
    uint32_t id() const { return packetId_; }
    const std::string& name() const { return packetName_; }
    
    /// Field access
    void addField(DecodedField field);
    
    std::size_t fieldCount() const { return fields_.size(); }
    
    const DecodedField* field(const std::string& name) const;
    const DecodedField* fieldAt(std::size_t index) const;
    
    /// Get all fields
    const std::vector<DecodedField>& fields() const { return fields_; }
    
    /// Convenience: get field value directly
    template<typename T>
    std::optional<T> get(const std::string& fieldName) const;
    
    /// Check if packet has a field
    bool hasField(const std::string& name) const;
    
    /// Iterator support
    auto begin() const { return fields_.begin(); }
    auto end() const { return fields_.end(); }

private:
    uint32_t packetId_ = 0;
    std::string packetName_;
    std::vector<DecodedField> fields_;
    std::unordered_map<std::string, std::size_t> fieldIndex_;
};

// --- Template implementations ---

template<typename T>
std::optional<T> DecodedField::as() const {
    const auto& val = hasScaling() ? scaledValue : rawValue;
    
    if (std::holds_alternative<T>(val)) {
        return std::get<T>(val);
    }
    
    // Try numeric conversions
    if constexpr (std::is_arithmetic_v<T>) {
        if (std::holds_alternative<int64_t>(val)) {
            return static_cast<T>(std::get<int64_t>(val));
        }
        if (std::holds_alternative<uint64_t>(val)) {
            return static_cast<T>(std::get<uint64_t>(val));
        }
        if (std::holds_alternative<double>(val)) {
            return static_cast<T>(std::get<double>(val));
        }
    }
    
    return std::nullopt;
}

template<typename T>
std::optional<T> DecodedPacket::get(const std::string& fieldName) const {
    const auto* f = field(fieldName);
    if (!f) {
        return std::nullopt;
    }
    return f->as<T>();
}

} // namespace ionet::codec

#endif
