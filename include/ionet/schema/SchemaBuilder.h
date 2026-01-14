#ifndef IONET_SCHEMA_BUILDER_H
#define IONET_SCHEMA_BUILDER_H

#include "Schema.h"
#include <stdexcept>

namespace ionet::schema {

/// Fluent builder for constructing schemas programmatically
class SchemaBuilder {
public:
    SchemaBuilder& name(std::string name) {
        info_.name = std::move(name);
        return *this;
    }
    
    SchemaBuilder& version(std::string version) {
        info_.version = std::move(version);
        return *this;
    }
    
    SchemaBuilder& description(std::string desc) {
        info_.description = std::move(desc);
        return *this;
    }
    
    SchemaBuilder& byteOrder(core::ByteOrder order) {
        byteOrder_ = order;
        return *this;
    }
    
    SchemaBuilder& bigEndian() {
        byteOrder_ = core::ByteOrder::Big;
        return *this;
    }
    
    SchemaBuilder& littleEndian() {
        byteOrder_ = core::ByteOrder::Little;
        return *this;
    }
    
    /// Start defining a new packet
    SchemaBuilder& packet(uint32_t id, std::string name) {
        finishCurrentPacket();
        currentPacket_ = Packet{};
        currentPacket_->id = id;
        currentPacket_->name = std::move(name);
        return *this;
    }
    
    SchemaBuilder& packetDescription(std::string desc) {
        ensurePacket();
        currentPacket_->description = std::move(desc);
        return *this;
    }
    
    /// Add a field to current packet
    SchemaBuilder& field(std::string name, core::DataType type) {
        ensurePacket();
        Field f;
        f.name = std::move(name);
        f.type = type;
        currentPacket_->fields.push_back(std::move(f));
        return *this;
    }
    
    /// Shorthand field methods
    SchemaBuilder& uint8(std::string name) { return field(std::move(name), core::DataType::UInt8); }
    SchemaBuilder& uint16(std::string name) { return field(std::move(name), core::DataType::UInt16); }
    SchemaBuilder& uint32(std::string name) { return field(std::move(name), core::DataType::UInt32); }
    SchemaBuilder& uint64(std::string name) { return field(std::move(name), core::DataType::UInt64); }
    SchemaBuilder& int8(std::string name) { return field(std::move(name), core::DataType::Int8); }
    SchemaBuilder& int16(std::string name) { return field(std::move(name), core::DataType::Int16); }
    SchemaBuilder& int32(std::string name) { return field(std::move(name), core::DataType::Int32); }
    SchemaBuilder& int64(std::string name) { return field(std::move(name), core::DataType::Int64); }
    SchemaBuilder& float32(std::string name) { return field(std::move(name), core::DataType::Float32); }
    SchemaBuilder& float64(std::string name) { return field(std::move(name), core::DataType::Float64); }
    
    /// Add scaling to last field
    SchemaBuilder& scaled(double scale, double offset = 0.0) {
        ensureField();
        currentPacket_->fields.back().scaling = Scaling{scale, offset};
        return *this;
    }
    
    /// Alias for scaled
    SchemaBuilder& scale(double scale) {
        return scaled(scale, 0.0);
    }
    
    /// Set offset for last field
    SchemaBuilder& offset(double off) {
        ensureField();
        if (!currentPacket_->fields.back().scaling) {
            currentPacket_->fields.back().scaling = Scaling{1.0, off};
        } else {
            currentPacket_->fields.back().scaling->offset = off;
        }
        return *this;
    }
    
    /// Add unit to last field
    SchemaBuilder& unit(std::string u) {
        ensureField();
        currentPacket_->fields.back().unit = std::move(u);
        return *this;
    }
    
    /// Add description to last field
    SchemaBuilder& fieldDescription(std::string desc) {
        ensureField();
        currentPacket_->fields.back().description = std::move(desc);
        return *this;
    }
    
    /// Add bitfield
    SchemaBuilder& bitfield(std::string name, uint8_t bits) {
        ensurePacket();
        Field f;
        f.name = std::move(name);
        f.type = core::DataType::Bitfield;
        f.bitCount = bits;
        currentPacket_->fields.push_back(std::move(f));
        return *this;
    }
    
    /// Add flag to last bitfield
    SchemaBuilder& flag(uint8_t bit, std::string name, std::string desc = "") {
        ensureField();
        auto& f = currentPacket_->fields.back();
        if (f.type != core::DataType::Bitfield) {
            throw std::logic_error("flag() can only be called after bitfield()");
        }
        f.bitFlags.push_back(BitFlag{bit, std::move(name), std::move(desc)});
        return *this;
    }
    
    /// Add a string field
    SchemaBuilder& string(std::string name, std::size_t size) {
        ensurePacket();
        Field f;
        f.name = std::move(name);
        f.type = core::DataType::String;
        f.stringSize = size;
        currentPacket_->fields.push_back(std::move(f));
        return *this;
    }
    
    /// Finish current field configuration (no-op, for fluent API)
    SchemaBuilder& done() {
        return *this;
    }
    
    /// Build the final schema
    Schema build() {
        finishCurrentPacket();
        
        Schema schema;
        schema.setInfo(std::move(info_));
        schema.setByteOrder(byteOrder_);
        
        for (auto& packet : packets_) {
            schema.addPacket(std::move(packet));
        }
        
        return schema;
    }

private:
    void ensurePacket() {
        if (!currentPacket_) {
            throw std::logic_error("No packet defined. Call packet() first.");
        }
    }
    
    void ensureField() {
        ensurePacket();
        if (currentPacket_->fields.empty()) {
            throw std::logic_error("No field defined. Call field() first.");
        }
    }
    
    void finishCurrentPacket() {
        if (currentPacket_) {
            packets_.push_back(std::move(*currentPacket_));
            currentPacket_.reset();
        }
    }
    
    SchemaInfo info_;
    core::ByteOrder byteOrder_ = core::ByteOrder::Big;
    std::vector<Packet> packets_;
    std::optional<Packet> currentPacket_;
};

} // namespace ionet::schema

#endif