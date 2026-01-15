#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <../include/ionet/codec/Decoder.h>
#include <../include/ionet/schema/SchemaLoader.h>

using namespace ionet::codec;
using namespace ionet::schema;
using namespace ionet::core;

const char* TEST_SCHEMA = R"(
schema:
  name: "TestSchema"
  version: "1.0"
  byte_order: "big"

packets:
  - id: 1
    name: "SimplePacket"
    fields:
      - name: "counter"
        type: "uint32"
      - name: "value"
        type: "int16"

  - id: 2
    name: "ScaledPacket"
    fields:
      - name: "temperature"
        type: "int16"
        scale: 0.01
        offset: -40.0
        unit: "celsius"
        min: -40.0
        max: 85.0
      - name: "voltage"
        type: "uint16"
        scale: 0.001
        unit: "volts"

  - id: 3
    name: "BitfieldPacket"
    fields:
      - name: "status"
        type: "bitfield"
        bits: 8
        flags:
          - { bit: 0, name: "active" }
          - { bit: 1, name: "error" }
          - { bit: 7, name: "ready" }
      - name: "mode"
        type: "uint8"

  - id: 4
    name: "AllTypesPacket"
    fields:
      - name: "i8"
        type: "int8"
      - name: "i16"
        type: "int16"
      - name: "i32"
        type: "int32"
      - name: "i64"
        type: "int64"
      - name: "u8"
        type: "uint8"
      - name: "u16"
        type: "uint16"
      - name: "u32"
        type: "uint32"
      - name: "u64"
        type: "uint64"
      - name: "f32"
        type: "float32"
      - name: "f64"
        type: "float64"

  - id: 5
    name: "StringPacket"
    fields:
      - name: "label"
        type: "string"
        size: 8
      - name: "id"
        type: "uint16"
)";

class DecoderFixture {
protected:
    DecoderFixture() {
        auto result = SchemaLoader::fromYaml(TEST_SCHEMA);
        REQUIRE(result.ok());
        schema_ = std::make_unique<Schema>(std::move(result.value()));
    }
    
    std::unique_ptr<Schema> schema_;
};

TEST_CASE_METHOD(DecoderFixture, "Decoder - decode simple packet", "[decoder]") {
    Decoder decoder(*schema_);
    
    // counter=0x12345678, value=0x00FF (255)
    std::vector<uint8_t> data = {
        0x12, 0x34, 0x56, 0x78,  // counter (big endian)
        0x00, 0xFF               // value (big endian)
    };
    
    auto result = decoder.decode(1, data);
    REQUIRE(result.ok());
    
    auto& packet = result.value();
    REQUIRE(packet.id() == 1);
    REQUIRE(packet.name() == "SimplePacket");
    REQUIRE(packet.fieldCount() == 2);
    
    auto counter = packet.get<uint64_t>("counter");
    REQUIRE(counter.has_value());
    REQUIRE(*counter == 0x12345678);
    
    auto value = packet.get<int64_t>("value");
    REQUIRE(value.has_value());
    REQUIRE(*value == 255);
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - decode with scaling", "[decoder]") {
    Decoder decoder(*schema_);
    
    // temperature: raw=6500 -> (6500 * 0.01) + (-40) = 25.0°C
    // voltage: raw=3300 -> 3300 * 0.001 = 3.3V
    std::vector<uint8_t> data = {
        0x19, 0x64,  // temperature = 6500 (big endian)
        0x0C, 0xE4   // voltage = 3300 (big endian)
    };
    
    auto result = decoder.decode(2, data);
    REQUIRE(result.ok());
    
    auto& packet = result.value();
    
    auto temp = packet.get<double>("temperature");
    REQUIRE(temp.has_value());
    REQUIRE_THAT(*temp, Catch::Matchers::WithinAbs(25.0, 0.001));
    
    auto voltage = packet.get<double>("voltage");
    REQUIRE(voltage.has_value());
    REQUIRE_THAT(*voltage, Catch::Matchers::WithinAbs(3.3, 0.001));
    
    // Check unit
    auto* tempField = packet.field("temperature");
    REQUIRE(tempField != nullptr);
    REQUIRE(tempField->unit == "celsius");
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - decode without scaling", "[decoder]") {
    DecodeOptions opts;
    opts.applyScaling = false;
    Decoder decoder(*schema_, opts);
    
    std::vector<uint8_t> data = {
        0x19, 0x64,  // temperature = 6500
        0x0C, 0xE4   // voltage = 3300
    };
    
    auto result = decoder.decode(2, data);
    REQUIRE(result.ok());
    
    auto& packet = result.value();
    
    // Should get raw value
    auto temp = packet.get<int64_t>("temperature");
    REQUIRE(temp.has_value());
    REQUIRE(*temp == 6500);
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - decode bitfield", "[decoder]") {
    Decoder decoder(*schema_);
    
    // status: 0b10000011 = active + error + ready
    // mode: 5
    std::vector<uint8_t> data = {
        0x83,  // status = 0b10000011
        0x05   // mode = 5
    };
    
    auto result = decoder.decode(3, data);
    REQUIRE(result.ok());
    
    auto& packet = result.value();
    
    auto* statusField = packet.field("status");
    REQUIRE(statusField != nullptr);
    REQUIRE(statusField->bitfield.has_value());
    
    auto& bf = *statusField->bitfield;
    REQUIRE(bf.rawValue == 0x83);
    REQUIRE(bf.isSet("active") == true);
    REQUIRE(bf.isSet("error") == true);
    REQUIRE(bf.isSet("ready") == true);
    REQUIRE(bf.bitAt(2) == false);
    
    auto mode = packet.get<uint64_t>("mode");
    REQUIRE(mode.has_value());
    REQUIRE(*mode == 5);
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - decode all types", "[decoder]") {
    Decoder decoder(*schema_);
    
    std::vector<uint8_t> data = {
        0xFF,                                // i8 = -1
        0xFF, 0xFE,                          // i16 = -2 (big endian)
        0xFF, 0xFF, 0xFF, 0xFD,              // i32 = -3 (big endian)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,  // i64 = -4 (big endian)
        0x01,                                // u8 = 1
        0x00, 0x02,                          // u16 = 2 (big endian)
        0x00, 0x00, 0x00, 0x03,              // u32 = 3 (big endian)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,  // u64 = 4 (big endian)
        0x40, 0x48, 0xF5, 0xC3,              // f32 = 3.14 (big endian IEEE754)
        0x40, 0x09, 0x21, 0xFB, 0x54, 0x44, 0x2D, 0x18   // f64 = pi (big endian)
    };
    
    auto result = decoder.decode(4, data);
    REQUIRE(result.ok());
    
    auto& packet = result.value();
    
    REQUIRE(*packet.get<int64_t>("i8") == -1);
    REQUIRE(*packet.get<int64_t>("i16") == -2);
    REQUIRE(*packet.get<int64_t>("i32") == -3);
    REQUIRE(*packet.get<int64_t>("i64") == -4);
    REQUIRE(*packet.get<uint64_t>("u8") == 1);
    REQUIRE(*packet.get<uint64_t>("u16") == 2);
    REQUIRE(*packet.get<uint64_t>("u32") == 3);
    REQUIRE(*packet.get<uint64_t>("u64") == 4);
    REQUIRE_THAT(*packet.get<double>("f32"), Catch::Matchers::WithinAbs(3.14, 0.01));
    REQUIRE_THAT(*packet.get<double>("f64"), Catch::Matchers::WithinAbs(3.14159265358979, 0.0000001));
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - decode string", "[decoder]") {
    Decoder decoder(*schema_);
    
    // label: "HELLO\0\0\0" (8 bytes), id: 42
    std::vector<uint8_t> data = {
        'H', 'E', 'L', 'L', 'O', 0, 0, 0,  // label
        0x00, 0x2A                          // id = 42 (big endian)
    };
    
    auto result = decoder.decode(5, data);
    REQUIRE(result.ok());
    
    auto& packet = result.value();
    
    auto* labelField = packet.field("label");
    REQUIRE(labelField != nullptr);
    REQUIRE(std::holds_alternative<std::string>(labelField->rawValue));
    
    auto label = std::get<std::string>(labelField->rawValue);
    REQUIRE(label.substr(0, 5) == "HELLO");
    
    REQUIRE(*packet.get<uint64_t>("id") == 42);
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - decode by name", "[decoder]") {
    Decoder decoder(*schema_);
    
    std::vector<uint8_t> data = {
        0x00, 0x00, 0x00, 0x01,  // counter
        0x00, 0x10               // value
    };
    
    auto result = decoder.decodeByName("SimplePacket", data);
    REQUIRE(result.ok());
    REQUIRE(result.value().name() == "SimplePacket");
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - unknown packet ID", "[decoder]") {
    Decoder decoder(*schema_);
    
    std::vector<uint8_t> data = {0x00};
    
    auto result = decoder.decode(999, data);
    REQUIRE(result.hasError());
    REQUIRE(result.error().message.find("Unknown packet ID") != std::string::npos);
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - unknown packet name", "[decoder]") {
    Decoder decoder(*schema_);
    
    std::vector<uint8_t> data = {0x00};
    
    auto result = decoder.decodeByName("NonExistent", data);
    REQUIRE(result.hasError());
    REQUIRE(result.error().message.find("Unknown packet name") != std::string::npos);
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - insufficient data", "[decoder]") {
    Decoder decoder(*schema_);
    
    // SimplePacket needs 6 bytes, only providing 2
    std::vector<uint8_t> data = {0x00, 0x01};
    
    auto result = decoder.decode(1, data);
    REQUIRE(result.hasError());
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - constraint violation", "[decoder]") {
    Decoder decoder(*schema_);
    
    // temperature: raw=20000 -> (20000 * 0.01) + (-40) = 160°C (exceeds max 85)
    std::vector<uint8_t> data = {
        0x4E, 0x20,  // temperature = 20000
        0x00, 0x00   // voltage = 0
    };
    
    auto result = decoder.decode(2, data);
    REQUIRE(result.hasError());
    REQUIRE(result.error().message.find("above maximum") != std::string::npos);
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - skip constraint validation", "[decoder]") {
    DecodeOptions opts;
    opts.validateConstraints = false;
    Decoder decoder(*schema_, opts);
    
    // temperature exceeds max but validation disabled
    std::vector<uint8_t> data = {
        0x4E, 0x20,  // temperature = 20000 -> 160°C
        0x00, 0x00   // voltage = 0
    };
    
    auto result = decoder.decode(2, data);
    REQUIRE(result.ok());
    
    auto temp = result.value().get<double>("temperature");
    REQUIRE_THAT(*temp, Catch::Matchers::WithinAbs(160.0, 0.001));
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - field iteration", "[decoder]") {
    Decoder decoder(*schema_);
    
    std::vector<uint8_t> data = {
        0x00, 0x00, 0x00, 0x01,
        0x00, 0x02
    };
    
    auto result = decoder.decode(1, data);
    REQUIRE(result.ok());
    
    auto& packet = result.value();
    
    std::vector<std::string> fieldNames;
    for (const auto& field : packet) {
        fieldNames.push_back(field.name);
    }
    
    REQUIRE(fieldNames.size() == 2);
    REQUIRE(fieldNames[0] == "counter");
    REQUIRE(fieldNames[1] == "value");
}

TEST_CASE_METHOD(DecoderFixture, "Decoder - hasField check", "[decoder]") {
    Decoder decoder(*schema_);
    
    std::vector<uint8_t> data = {
        0x00, 0x00, 0x00, 0x01,
        0x00, 0x02
    };
    
    auto result = decoder.decode(1, data);
    REQUIRE(result.ok());
    
    auto& packet = result.value();
    
    REQUIRE(packet.hasField("counter") == true);
    REQUIRE(packet.hasField("value") == true);
    REQUIRE(packet.hasField("nonexistent") == false);
}
