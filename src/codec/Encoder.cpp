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
        if (!packet.hasField(field.name)) {
            return ionet::core::Result<std::vector<uint8_t>>(ionet::core::Error("Missing field: " + field.name));
        }

        const auto& value = packet.rawValue(field.name);

        // Constraint validation (if not skipped)
        if (!impl_->options.skipValidation) {
            double scaledValue = 0.0;
            bool hasScaling = field.scaling.has_value();
            if (hasScaling) {
                double raw = 0.0;
                if (std::holds_alternative<int64_t>(value)) {
                    raw = static_cast<double>(std::get<int64_t>(value));
                } else if (std::holds_alternative<uint64_t>(value)) {
                    raw = static_cast<double>(std::get<uint64_t>(value));
                } else {
                    return ionet::core::Result<std::vector<uint8_t>>(ionet::core::Error("Invalid value type for scaled field: " + field.name));
                }
                scaledValue = raw * field.scaling->scale + field.scaling->offset;
            } else {
                if (std::holds_alternative<double>(value)) {
                    scaledValue = std::get<double>(value);
                } else if (std::holds_alternative<int64_t>(value)) {
                    scaledValue = static_cast<double>(std::get<int64_t>(value));
                } else if (std::holds_alternative<uint64_t>(value)) {
                    scaledValue = static_cast<double>(std::get<uint64_t>(value));
                } // For strings/bitfields, skip scaling check
            }

            if (field.constraints.min && scaledValue < *field.constraints.min) {
                return ionet::core::Result<std::vector<uint8_t>>(ionet::core::Error("Value below minimum for field: " + field.name));
            }
            if (field.constraints.max && scaledValue > *field.constraints.max) {
                return ionet::core::Result<std::vector<uint8_t>>(ionet::core::Error("Value above maximum for field: " + field.name));
            }
        }

        // Encode based on type
        if (field.type == ionet::core::DataType::UInt8) {
            uint8_t v = static_cast<uint8_t>(std::get<uint64_t>(value));
            buffer.push_back(v);
        } else if (field.type == ionet::core::DataType::Int8) {
            int8_t v = static_cast<int8_t>(std::get<int64_t>(value));
            buffer.push_back(static_cast<uint8_t>(v));
        } else if (field.type == ionet::core::DataType::UInt16) {
            uint16_t v = static_cast<uint16_t>(std::get<uint64_t>(value));
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == ionet::core::DataType::Int16) {
            int16_t v = static_cast<int16_t>(std::get<int64_t>(value));
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == ionet::core::DataType::UInt32) {
            uint32_t v = static_cast<uint32_t>(std::get<uint64_t>(value));
            buffer.push_back((v >> 24) & 0xFF);
            buffer.push_back((v >> 16) & 0xFF);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == ionet::core::DataType::Int32) {
            int32_t v = static_cast<int32_t>(std::get<int64_t>(value));
            buffer.push_back((v >> 24) & 0xFF);
            buffer.push_back((v >> 16) & 0xFF);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == ionet::core::DataType::UInt64) {
            uint64_t v = std::get<uint64_t>(value);
            for (int i = 7; i >= 0; --i)
                buffer.push_back((v >> (i * 8)) & 0xFF);
        } else if (field.type == ionet::core::DataType::Int64) {
            int64_t v = std::get<int64_t>(value);
            for (int i = 7; i >= 0; --i)
                buffer.push_back((v >> (i * 8)) & 0xFF);
        } else if (field.type == ionet::core::DataType::Float32) {
            float f = static_cast<float>(std::get<double>(value));
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
        } else if (field.type == ionet::core::DataType::Bitfield) {
            uint64_t v = std::get<uint64_t>(value);
            uint8_t bitCount = field.bitCount.value_or(8);
            if (bitCount <= 8) {
                buffer.push_back(v & 0xFF);
            } else if (bitCount <= 16) {
                buffer.push_back((v >> 8) & 0xFF);
                buffer.push_back(v & 0xFF);
            } else if (bitCount <= 32) {
                buffer.push_back((v >> 24) & 0xFF);
                buffer.push_back((v >> 16) & 0xFF);
                buffer.push_back((v >> 8) & 0xFF);
                buffer.push_back(v & 0xFF);
            } else {
                for (int i = 7; i >= 0; --i)
                    buffer.push_back((v >> (i * 8)) & 0xFF);
            }
        } else if (field.type == ionet::core::DataType::String) {
            std::string str = std::get<std::string>(value);
            std::size_t size = field.stringSize.value_or(str.size());
            buffer.insert(buffer.end(), str.begin(), str.end());
            if (size > str.size()) {
                buffer.insert(buffer.end(), size - str.size(), 0);
            }
        } else {
            return ionet::core::Result<std::vector<uint8_t>>(ionet::core::Error("Unsupported field type"));
        }
    }

    return ionet::core::Result<std::vector<uint8_t>>(std::move(buffer));
}

} // namespace codec
} // namespace ionet