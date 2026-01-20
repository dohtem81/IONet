#include <../include/ionet/codec/Encoder.h>
#include <../include/ionet/schema/Schema.h>
#include <../include/ionet/schema/Packet.h>
#include <../include/ionet/core/Result.h>
#include <cstring>
#include <stdexcept>

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
        return ionet::core::Result<std::vector<uint8_t>>::error(
            ionet::core::Result<std::vector<uint8_t>>::ErrorType{"Unknown packet ID: " + std::to_string(packet.id)}
        );
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(64); // heuristic

    for (const auto& field : pktDef->fields) {
        if (!packet.hasField(field.name)) {
            return ionet::core::Result<std::vector<uint8_t>>::error("Missing field: " + field.name);
        }

        const auto& value = packet.rawValue(field.name);

        // Handle each type
        if (field.type == "uint8") {
            buffer.push_back(static_cast<uint8_t>(std::get<uint64_t>(value)));
        } else if (field.type == "int8") {
            buffer.push_back(static_cast<uint8_t>(std::get<int64_t>(value)));
        } else if (field.type == "uint16") {
            uint16_t v = static_cast<uint16_t>(std::get<uint64_t>(value));
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == "int16") {
            int16_t v = static_cast<int16_t>(std::get<int64_t>(value));
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == "uint32") {
            uint32_t v = static_cast<uint32_t>(std::get<uint64_t>(value));
            buffer.push_back((v >> 24) & 0xFF);
            buffer.push_back((v >> 16) & 0xFF);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == "int32") {
            int32_t v = static_cast<int32_t>(std::get<int64_t>(value));
            buffer.push_back((v >> 24) & 0xFF);
            buffer.push_back((v >> 16) & 0xFF);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == "uint64") {
            uint64_t v = std::get<uint64_t>(value);
            for (int i = 7; i >= 0; --i)
                buffer.push_back((v >> (i * 8)) & 0xFF);
        } else if (field.type == "int64") {
            int64_t v = std::get<int64_t>(value);
            for (int i = 7; i >= 0; --i)
                buffer.push_back((v >> (i * 8)) & 0xFF);
        } else if (field.type == "float32") {
            float f = std::get<double>(value);
            uint32_t v;
            std::memcpy(&v, &f, sizeof(float));
            buffer.push_back((v >> 24) & 0xFF);
            buffer.push_back((v >> 16) & 0xFF);
            buffer.push_back((v >> 8) & 0xFF);
            buffer.push_back(v & 0xFF);
        } else if (field.type == "float64") {
            double d = std::get<double>(value);
            uint64_t v;
            std::memcpy(&v, &d, sizeof(double));
            for (int i = 7; i >= 0; --i)
                buffer.push_back((v >> (i * 8)) & 0xFF);
        } else if (field.type == "string") {
            const std::string& str = std::get<std::string>(value);
            size_t sz = field.size.value_or(0);
            for (size_t i = 0; i < sz; ++i) {
                if (i < str.size())
                    buffer.push_back(str[i]);
                else
                    buffer.push_back(0);
            }
        } else if (field.type == "bitfield") {
            // Assume bitfield is stored as uint8_t in value
            buffer.push_back(static_cast<uint8_t>(std::get<uint64_t>(value)));
        } else {
            return ionet::core::Result<std::vector<uint8_t>>::error("Unsupported field type: " + field.type);
        }
    }

    return ionet::core::Result<std::vector<uint8_t>>::ok(std::move(buffer));
}

} // namespace codec
} // namespace ionet