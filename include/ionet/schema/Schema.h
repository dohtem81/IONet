#ifndef IONET_SCHEMA_SCHEMA_H
#define IONET_SCHEMA_SCHEMA_H

#include "Packet.h"
#include "../core/Types.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace ionet::schema {

/// Schema metadata
struct SchemaInfo {
    std::string name;
    std::string version;
    std::string description;
};

/// Complete schema definition
class Schema {
public:
    Schema() = default;
    
    // Metadata
    void setInfo(SchemaInfo info) { info_ = std::move(info); }
    const SchemaInfo& info() const { return info_; }
    
    // Byte order
    void setByteOrder(core::ByteOrder order) { byteOrder_ = order; }
    core::ByteOrder byteOrder() const { return byteOrder_; }
    
    // Packet management
    void addPacket(Packet packet) {
        uint32_t id = packet.id;
        std::string name = packet.name;
        packets_.push_back(std::move(packet));
        idIndex_[id] = packets_.size() - 1;
        nameIndex_[name] = packets_.size() - 1;
    }
    
    /// Get all packets
    const std::vector<Packet>& packets() const { return packets_; }
    
    /// Find packet by ID
    const Packet* findPacketById(uint32_t id) const {
        auto it = idIndex_.find(id);
        if (it != idIndex_.end()) {
            return &packets_[it->second];
        }
        return nullptr;
    }
    
    /// Find packet by name
    const Packet* findPacketByName(const std::string& name) const {
        auto it = nameIndex_.find(name);
        if (it != nameIndex_.end()) {
            return &packets_[it->second];
        }
        return nullptr;
    }
    
    /// Check if schema has any packets
    bool empty() const { return packets_.empty(); }
    
    /// Get packet count
    std::size_t packetCount() const { return packets_.size(); }
    
    /// Validate schema integrity
    bool validate(std::string* errorOut = nullptr) const {
        // Check for duplicate IDs
        if (idIndex_.size() != packets_.size()) {
            if (errorOut) *errorOut = "Duplicate packet IDs detected";
            return false;
        }
        
        // Check for duplicate names
        if (nameIndex_.size() != packets_.size()) {
            if (errorOut) *errorOut = "Duplicate packet names detected";
            return false;
        }
        
        // Check each packet has at least one field
        for (const auto& packet : packets_) {
            if (packet.fields.empty()) {
                if (errorOut) *errorOut = "Packet '" + packet.name + "' has no fields";
                return false;
            }
        }
        
        return true;
    }

private:
    SchemaInfo info_;
    core::ByteOrder byteOrder_ = core::ByteOrder::Big;
    std::vector<Packet> packets_;
    std::unordered_map<uint32_t, std::size_t> idIndex_;
    std::unordered_map<std::string, std::size_t> nameIndex_;
};

} // namespace ionet::schema

#endif