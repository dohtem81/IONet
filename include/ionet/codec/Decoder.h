#ifndef IONET_CODEC_DECODER_H
#define IONET_CODEC_DECODER_H

#include "DecodedPacket.h"
#include "../schema/Schema.h"
#include "../core/ByteBuffer.h"
#include <memory>
#include <span>
#include <optional>
#include <variant>

namespace ionet::codec {

/// Error information from decoding
struct DecodeError {
    std::string message;
    std::size_t byteOffset = 0;
    std::string fieldName;
};

/// Result type for decode operations (C++20 compatible alternative to std::expected)
template<typename T, typename E>
class Result {
public:
    Result(T value) : data_(std::move(value)) {}
    Result(E error) : data_(std::move(error)) {}
    
    bool has_value() const { return std::holds_alternative<T>(data_); }
    explicit operator bool() const { return has_value(); }
    
    const T& value() const { return std::get<T>(data_); }
    T& value() { return std::get<T>(data_); }
    
    const E& error() const { return std::get<E>(data_); }
    E& error() { return std::get<E>(data_); }
    
private:
    std::variant<T, E> data_;
};

using DecodeResult = Result<DecodedPacket, DecodeError>;

/// Decoder for parsing binary data according to schemas
class Decoder {
public:
    explicit Decoder(const schema::Schema& schema)
        : schema_(schema) {}
    
    /// Decode a packet by ID
    DecodeResult decode(uint32_t packetId, std::span<const uint8_t> data) const;
    
    /// Decode a packet by name
    DecodeResult decodeByName(const std::string& packetName, std::span<const uint8_t> data) const;
    
private:
    const schema::Schema& schema_;
    
    // Helper methods
    FieldValue decodeField(const schema::Field& field, core::ByteBuffer& buffer) const;
    uint64_t decodeBitfield(const schema::Field& field, core::ByteBuffer& buffer) const;
    BitfieldValue extractBitFlags(uint64_t raw, const std::vector<schema::BitFlag>& flags) const;
    bool validateField(const FieldValue& value, const schema::Field& field) const;
};

} // namespace ionet::codec

#endif
