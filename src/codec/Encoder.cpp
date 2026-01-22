#include <../include/ionet/codec/Encoder.h>
#include <../include/ionet/schema/Schema.h>
#include <../include/ionet/schema/Packet.h>
#include <../include/ionet/core/Result.h>
#include <cstring>
#include <stdexcept>
#include <iostream>

namespace ionet {
namespace codec {

struct Encoder::Impl {
    const schema::Schema& schema;
    EncodeOptions options;

    Impl(const schema::Schema& s, EncodeOptions opts)
        : schema(s), options(opts) {}
};

Encoder::Encoder(const schema::Schema& schema, EncodeOptions opts)
    : impl_(std::make_unique<Impl>(schema, opts)) {}

Encoder::~Encoder() = default;

ionet::core::Result<std::vector<uint8_t>> Encoder::encode(const ionet::schema::Packet& packet) const {
    // Find packet definition in schema
    const auto* pktDef = impl_->schema.findPacketById(packet.id);
    if (!pktDef) {
        return ionet::core::Result<std::vector<uint8_t>>(ionet::core::Error("Packet ID not found in schema: " + std::to_string(packet.id)));
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(64); // heuristic

    for (const auto& field : pktDef->fields) {
        const auto& value = packet.rawValue(field.name);

        // Handle each type
        if (field.type == ionet::core::DataType::UInt8) {
            uint64_t raw = std::get<uint64_t>(value);
            uint8_t v = static_cast<uint8_t>(raw);
            buffer.push_back(v);
        } else if (field.type == ionet::core::DataType::Int8) {
            uint64_t raw = std::get<uint64_t>(value);
            int8_t v = static_cast<int8_t>(raw);
            buffer.push_back(static_cast<uint8_t>(v));
        } else if (field.type == ionet::core::DataType::UInt16) {
            uint64_t raw = std::get<uint64_t>(value);
            uint16_t v = static_cast<uint16_t>(raw);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == ionet::core::DataType::Int16) {
            uint64_t raw = std::get<uint64_t>(value);
            int16_t v = static_cast<int16_t>(raw);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == ionet::core::DataType::UInt32) {
            uint64_t raw = std::get<uint64_t>(value);
            uint32_t v = static_cast<uint32_t>(raw);
            buffer.push_back((v >> 24) & 0xFF);
            buffer.push_back((v >> 16) & 0xFF);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == ionet::core::DataType::Int32) {
            uint64_t raw = std::get<uint64_t>(value);
            int32_t v = static_cast<int32_t>(raw);
            buffer.push_back((v >> 24) & 0xFF);
            buffer.push_back((v >> 16) & 0xFF);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == ionet::core::DataType::UInt64) {
            uint64_t v = std::get<uint64_t>(value);
            for (int i = 7; i >= 0; --i)
                buffer.push_back((v >> (i * 8)) & 0xFF);
        } else if (field.type == ionet::core::DataType::Int64) {
            uint64_t raw = std::get<uint64_t>(value);
            int64_t v = static_cast<int64_t>(raw);
            for (int i = 7; i >= 0; --i)
                buffer.push_back((v >> (i * 8)) & 0xFF);
        } else if (field.type == ionet::core::DataType::Float32) {
            double d = std::get<double>(value);
            float f = static_cast<float>(d);
            uint32_t v;
            std::memcpy(&v, &f, sizeof(float));
            buffer.push_back((v >> 24) & 0xFF);
            buffer.push_back((v >> 16) & 0xFF);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == ionet::core::DataType::Float64) {
            double d = std::get<double>(value);
            uint64_t v;
            std::memcpy(&v, &d, sizeof(double));
            for (int i = 7; i >= 0; --i)
                buffer.push_back((v >> (i * 8)) & 0xFF);
        } else if (field.type == ionet::core::DataType::String) {
            std::string str = std::get<std::string>(value);
            size_t sz = field.stringSize.value_or(0);
            for (size_t i = 0; i < sz; ++i) {
                if (i < str.size())
                    buffer.push_back(static_cast<uint8_t>(str[i]));
                else
                    buffer.push_back(0);
            }
        } else {
            return ionet::core::Result<std::vector<uint8_t>>(ionet::core::Error("Unsupported field type"));
        }
    }

    return ionet::core::Result<std::vector<uint8_t>>(std::move(buffer));
}

} // namespace codec
} // namespace ionet