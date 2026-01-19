#pragma once

#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <ionet/schema/Schema.h>
#include <ionet/core/Packet.h>
#include <ionet/core/Result.h>

namespace ionet {
namespace codec {

struct EncodeOptions {
    bool validateConstraints = true;
    // Add more options as needed
};

class Encoder {
public:
    Encoder(const ionet::schema::Schema& schema, EncodeOptions opts = {});
    ~Encoder();

    // Encode by packet ID or name (from Packet)
    ionet::core::Result<std::vector<uint8_t>> encode(const ionet::core::Packet& packet) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace codec
} // namespace ionet