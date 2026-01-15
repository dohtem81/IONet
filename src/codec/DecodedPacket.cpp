#include "../../include/ionet/codec/DecodedPacket.h"

namespace ionet::codec {

// --- DecodedBitfield ---

bool DecodedBitfield::isSet(const std::string& flagName) const {
    auto it = flags.find(flagName);
    if (it != flags.end()) {
        return it->second;
    }
    return false;
}

bool DecodedBitfield::bitAt(uint8_t bit) const {
    return (rawValue >> bit) & 1;
}

// --- DecodedField ---

core::Value DecodedField::value() const {
    if (hasScaling()) {
        return scaledValue;
    }
    return rawValue;
}

bool DecodedField::hasScaling() const {
    // Check if scaledValue is different from rawValue
    // This is true if scaling was applied
    return !std::holds_alternative<std::monostate>(scaledValue) &&
           scaledValue.index() != rawValue.index();
}

// --- DecodedPacket ---

DecodedPacket::DecodedPacket(uint32_t packetId, std::string packetName)
    : packetId_(packetId)
    , packetName_(std::move(packetName)) 
{}

void DecodedPacket::addField(DecodedField field) {
    fieldIndex_[field.name] = fields_.size();
    fields_.push_back(std::move(field));
}

const DecodedField* DecodedPacket::field(const std::string& name) const {
    auto it = fieldIndex_.find(name);
    if (it != fieldIndex_.end()) {
        return &fields_[it->second];
    }
    return nullptr;
}

const DecodedField* DecodedPacket::fieldAt(std::size_t index) const {
    if (index < fields_.size()) {
        return &fields_[index];
    }
    return nullptr;
}

bool DecodedPacket::hasField(const std::string& name) const {
    return fieldIndex_.find(name) != fieldIndex_.end();
}

} // namespace ionet::codec