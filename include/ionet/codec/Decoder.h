#ifndef IONET_CODEC_DECODER_H
#define IONET_CODEC_DECODER_H

#include "DecodedPacket.h"
#include "../core/Result.h"
#include "../core/ByteBuffer.h"
#include "../schema/Schema.h"
#include <span>
#include <cstdint>

namespace ionet::codec {

/// Options for decoding behavior
struct DecodeOptions {
    /// Apply scaling to values (default: true)
    bool applyScaling = true;
    
    /// Validate constraints after decoding (default: true)
    bool validateConstraints = true;
    
    /// Stop on first error vs collect all errors (default: true)
    bool stopOnError = true;
};

/// Decoder for binary data using schema definitions
class Decoder {
public:
    /// Construct decoder with schema reference
    explicit Decoder(const schema::Schema& schema);
    
    /// Construct decoder with schema and options
    Decoder(const schema::Schema& schema, DecodeOptions options);
    
    /// Decode a packet by ID from raw bytes
    core::Result<DecodedPacket> decode(
        uint32_t packetId,
        std::span<const uint8_t> data
    ) const;
    
    /// Decode a packet by ID from vector
    core::Result<DecodedPacket> decode(
        uint32_t packetId,
        const std::vector<uint8_t>& data
    ) const;
    
    /// Decode a packet by name from raw bytes
    core::Result<DecodedPacket> decodeByName(
        const std::string& packetName,
        std::span<const uint8_t> data
    ) const;
    
    /// Decode using ByteBufferReader (for streaming)
    core::Result<DecodedPacket> decode(
        uint32_t packetId,
        core::ByteBufferReader& reader
    ) const;
    
    /// Get/set options
    const DecodeOptions& options() const { return options_; }
    void setOptions(DecodeOptions options) { options_ = options; }
    
    /// Get schema reference
    const schema::Schema& schema() const { return schema_; }

private:
    const schema::Schema& schema_;
    DecodeOptions options_;
    
    /// Decode a single field
    core::Result<DecodedField> decodeField(
        const schema::Field& fieldDef,
        core::ByteBufferReader& reader,
        core::ByteOrder byteOrder
    ) const;
    
    /// Decode a bitfield
    DecodedBitfield decodeBitfield(
        uint64_t rawValue,
        const schema::Field& fieldDef
    ) const;
    
    /// Apply scaling to a value
    core::Value applyScaling(
        const core::Value& rawValue,
        const schema::Scaling& scaling
    ) const;
    
    /// Validate field constraints
    bool validateConstraints(
        const DecodedField& field,
        const schema::Field& fieldDef,
        std::string* errorMsg
    ) const;
};

} // namespace ionet::codec

#endif
