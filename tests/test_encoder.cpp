#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <../include/ionet/codec/Encoder.h>
#include <../include/ionet/codec/Decoder.h>
#include <../include/ionet/schema/SchemaLoader.h>

using namespace ionet::codec;
using namespace ionet::schema;
using namespace ionet::core;

const char* ENCODER_TEST_SCHEMA = R"(
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
        offset: 40.0
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

class EncoderFixture {
protected:
    EncoderFixture() {
        auto result = SchemaLoader::fromYaml(ENCODER_TEST_SCHEMA);
        REQUIRE(result.ok());
        schema_ = std::make_unique<Schema>(std::move(result.value()));
    }
    
    std::unique_ptr<Schema> schema_;
};

TEST_CASE_METHOD(EncoderFixture, "Encoder - list fields in hard coded packet", "[encoder]") {
    Packet packet(1, "SimplePacket");
    packet.add("counter", uint32_t(0x12345678));
    packet.add("value", int16_t(255));

    std::vector<std::string> expected = {
      "value",
      "counter"
    };
    auto result = packet.listFields();
    REQUIRE(result == expected);
  }

TEST_CASE_METHOD(EncoderFixture, "Encoder - list fields in simple packet", "[encoder]") {
    auto* schPacket = schema_->findPacketById(1);
    Packet* packet = schPacket ? new Packet(*schPacket) : nullptr;

    auto pktName = packet->name;
    REQUIRE(packet != nullptr);
    REQUIRE(pktName == "SimplePacket");

    auto* counterField = packet->findField("counter");
    REQUIRE(counterField != nullptr);
    REQUIRE(counterField->type == DataType::UInt32);
    auto* valueField = packet->findField("value");
    REQUIRE(valueField != nullptr);
    REQUIRE(valueField->type == DataType::Int16);

    packet->set("counter", uint32_t(0x00));
    packet->set("value", int16_t(0x00));    

    auto fieldList = packet->listFields();
    std::vector<std::string> expected = {
      "value",
      "counter"
    };
    REQUIRE(fieldList == expected);
  }  

TEST_CASE_METHOD(EncoderFixture, "Encoder - encode simple packet", "[encoder]") {
    Encoder encoder(*schema_);
    Decoder decoder(*schema_);

    const auto* constPacket = schema_->findPacketById(1);
    auto pktName = constPacket->name;
    REQUIRE(constPacket != nullptr);
    REQUIRE(pktName == "SimplePacket");

    Packet packet = *constPacket;
    auto* counterField = packet.findField("counter");
    REQUIRE(counterField != nullptr);
    REQUIRE(counterField->type == DataType::UInt32);
    auto* valueField = packet.findField("value");
    REQUIRE(valueField != nullptr);
    REQUIRE(valueField->type == DataType::Int16);
     
    packet.set("counter", uint32_t(0x12345678));
    packet.set("value", int16_t(255));

    auto fieldSize = packet.fields.size();
    REQUIRE(fieldSize == 2);

    auto resultEncoding = encoder.encode(packet);
    REQUIRE(resultEncoding.ok());
    // Expected encoded bytes: counter (0x12345678), value (0x00FF)
    std::vector<uint8_t> expected = {
        0x12, 0x34, 0x56, 0x78,  // counter (big endian)
        0x00, 0xFF               // value (big endian)
    };
    REQUIRE(resultEncoding.value() == expected);

    // Round-trip decode
    auto decodeResult = decoder.decode(1, resultEncoding.value());
    REQUIRE(decodeResult.ok());
    auto& decoded = decodeResult.value();
    REQUIRE(*decoded.get<uint64_t>("counter") == 0x12345678);
    REQUIRE(*decoded.get<int64_t>("value") == 255);
}

TEST_CASE_METHOD(EncoderFixture, "Encoder - encode with scaling", "[encoder]") {
    Encoder encoder(*schema_);

    const auto* pktDef = schema_->findPacketById(2);  // Assuming ID 2 for scaled packet
    REQUIRE(pktDef != nullptr);
    Packet packet = *pktDef;

    packet.set("temperature", 25.0);
    packet.set("voltage", 3.3);

    auto result = encoder.encode(packet);
    REQUIRE(result.ok());

    // Round-trip decode with scaling enabled
    DecodeOptions decodeOpts;
    decodeOpts.applyScaling = true;
    Decoder decoder(*schema_, decodeOpts);
    auto decodeResult = decoder.decode(2, result.value());
    REQUIRE(decodeResult.ok());  // Should now pass
    auto& decoded = decodeResult.value();
    REQUIRE_THAT(*decoded.get<double>("temperature"), Catch::Matchers::WithinAbs(25.0, 0.001));
    REQUIRE_THAT(*decoded.get<double>("voltage"), Catch::Matchers::WithinAbs(3.3, 0.001));
}

// TEST_CASE_METHOD(EncoderFixture, "Encoder - encode bitfield", "[encoder]") {
//     Encoder encoder(*schema_);
//     Decoder decoder(*schema_);

//     Packet packet(3, "BitfieldPacket");
//     packet.set("status", uint8_t(0x83)); // active, error, ready
//     packet.set("mode", uint8_t(5));

//     auto result = encoder.encode(packet);
//     REQUIRE(result.ok());

//     std::vector<uint8_t> expected = {
//         0x83,  // status
//         0x05   // mode
//     };
//     REQUIRE(result.value() == expected);

//     // Round-trip decode
//     auto decodeResult = decoder.decode(3, result.value());
//     REQUIRE(decodeResult.ok());
//     auto& decoded = decodeResult.value();
//     auto* statusField = decoded.field("status");
//     REQUIRE(statusField != nullptr);
//     REQUIRE(statusField->bitfield.has_value());
//     auto& bf = *statusField->bitfield;
//     REQUIRE(bf.rawValue == 0x83);
//     REQUIRE(bf.isSet("active") == true);
//     REQUIRE(bf.isSet("error") == true);
//     REQUIRE(bf.isSet("ready") == true);
//     REQUIRE(*decoded.get<uint64_t>("mode") == 5);
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - encode all types", "[encoder]") {
//     Encoder encoder(*schema_);
//     Decoder decoder(*schema_);

//     Packet packet(4, "AllTypesPacket");
//     packet.set("i8", int8_t(-1));
//     packet.set("i16", int16_t(-2));
//     packet.set("i32", int32_t(-3));
//     packet.set("i64", int64_t(-4));
//     packet.set("u8", uint8_t(1));
//     packet.set("u16", uint16_t(2));
//     packet.set("u32", uint32_t(3));
//     packet.set("u64", uint64_t(4));
//     packet.set("f32", float(3.14));
//     packet.set("f64", double(3.14159265358979));

//     auto result = encoder.encode(packet);
//     REQUIRE(result.ok());

//     std::vector<uint8_t> expected = {
//         0xFF,                                // i8 = -1
//         0xFF, 0xFE,                          // i16 = -2
//         0xFF, 0xFF, 0xFF, 0xFD,              // i32 = -3
//         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,  // i64 = -4
//         0x01,                                // u8 = 1
//         0x00, 0x02,                          // u16 = 2
//         0x00, 0x00, 0x00, 0x03,              // u32 = 3
//         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,  // u64 = 4
//         0x40, 0x48, 0xF5, 0xC3,              // f32 = 3.14
//         0x40, 0x09, 0x21, 0xFB, 0x54, 0x44, 0x2D, 0x18   // f64 = pi
//     };
//     REQUIRE(result.value() == expected);

//     // Round-trip decode
//     auto decodeResult = decoder.decode(4, result.value());
//     REQUIRE(decodeResult.ok());
//     auto& decoded = decodeResult.value();
//     REQUIRE(*decoded.get<int64_t>("i8") == -1);
//     REQUIRE(*decoded.get<int64_t>("i16") == -2);
//     REQUIRE(*decoded.get<int64_t>("i32") == -3);
//     REQUIRE(*decoded.get<int64_t>("i64") == -4);
//     REQUIRE(*decoded.get<uint64_t>("u8") == 1);
//     REQUIRE(*decoded.get<uint64_t>("u16") == 2);
//     REQUIRE(*decoded.get<uint64_t>("u32") == 3);
//     REQUIRE(*decoded.get<uint64_t>("u64") == 4);
//     REQUIRE_THAT(*decoded.get<double>("f32"), Catch::Matchers::WithinAbs(3.14, 0.01));
//     REQUIRE_THAT(*decoded.get<double>("f64"), Catch::Matchers::WithinAbs(3.14159265358979, 0.0000001));
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - encode string", "[encoder]") {
//     Encoder encoder(*schema_);
//     Decoder decoder(*schema_);

//     Packet packet(5, "StringPacket");
//     packet.set("label", std::string("HELLO"));
//     packet.set("id", uint16_t(42));

//     auto result = encoder.encode(packet);
//     REQUIRE(result.ok());

//     std::vector<uint8_t> expected = {
//         'H', 'E', 'L', 'L', 'O', 0, 0, 0,  // label padded to 8 bytes
//         0x00, 0x2A                          // id = 42
//     };
//     REQUIRE(result.value() == expected);

//     // Round-trip decode
//     auto decodeResult = decoder.decode(5, result.value());
//     REQUIRE(decodeResult.ok());
//     auto& decoded = decodeResult.value();
//     auto* labelField = decoded.field("label");
//     REQUIRE(labelField != nullptr);
//     REQUIRE(std::holds_alternative<std::string>(labelField->rawValue));
//     auto label = std::get<std::string>(labelField->rawValue);
//     REQUIRE(label.substr(0, 5) == "HELLO");
//     REQUIRE(*decoded.get<uint64_t>("id") == 42);
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - encode by name", "[encoder]") {
//     Encoder encoder(*schema_);
//     Decoder decoder(*schema_);

//     Packet packet(1, "SimplePacket");
//     packet.set("counter", uint32_t(1));
//     packet.set("value", int16_t(16));

//     auto result = encoder.encode(packet);
//     REQUIRE(result.ok());

//     std::vector<uint8_t> expected = {
//         0x00, 0x00, 0x00, 0x01,  // counter
//         0x00, 0x10               // value
//     };
//     REQUIRE(result.value() == expected);

//     // Round-trip decode by name
//     auto decodeResult = decoder.decodeByName("SimplePacket", result.value());
//     REQUIRE(decodeResult.ok());
//     REQUIRE(decodeResult.value().name() == "SimplePacket");
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - unknown packet ID", "[encoder]") {
//     Encoder encoder(*schema_);

//     Packet packet(999, "UnknownPacket");
//     packet.set("field", uint8_t(0));

//     auto result = encoder.encode(packet);
//     REQUIRE(result.hasError());
//     REQUIRE(result.error().message.find("Unknown packet ID") != std::string::npos);
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - unknown packet name", "[encoder]") {
//     Encoder encoder(*schema_);

//     Packet packet(0, "NonExistent");
//     packet.set("field", uint8_t(0));

//     auto result = encoder.encode(packet);
//     REQUIRE(result.hasError());
//     REQUIRE(result.error().message.find("Unknown packet name") != std::string::npos);
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - insufficient fields", "[encoder]") {
//     Encoder encoder(*schema_);

//     Packet packet(1, "SimplePacket");
//     packet.set("counter", uint32_t(1)); // missing "value"

//     auto result = encoder.encode(packet);
//     REQUIRE(result.hasError());
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - constraint violation", "[encoder]") {
//     Encoder encoder(*schema_);

//     Packet packet(2, "ScaledPacket");
//     packet.set("temperature", 160.0); // exceeds max
//     packet.set("voltage", 0.0);

//     auto result = encoder.encode(packet);
//     REQUIRE(result.hasError());
//     REQUIRE(result.error().message.find("above maximum") != std::string::npos);
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - skip constraint validation", "[encoder]") {
//     EncodeOptions opts;
//     opts.validateConstraints = false;
//     Encoder encoder(*schema_, opts);

//     Packet packet(2, "ScaledPacket");
//     packet.set("temperature", 160.0); // exceeds max, but validation disabled
//     packet.set("voltage", 0.0);

//     auto result = encoder.encode(packet);
//     REQUIRE(result.ok());

//     // Should encode raw value
//     std::vector<uint8_t> expected = {
//         0x4E, 0x20,  // temperature = 20000
//         0x00, 0x00   // voltage = 0
//     };
//     REQUIRE(result.value() == expected);
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - field iteration", "[encoder]") {
//     Encoder encoder(*schema_);

//     Packet packet(1, "SimplePacket");
//     packet.set("counter", uint32_t(1));
//     packet.set("value", int16_t(2));

//     std::vector<std::string> fieldNames;
//     for (const auto& field : packet.fields) {
//         fieldNames.push_back(field.name);
//     }

//     REQUIRE(fieldNames.size() == 2);
//     REQUIRE(fieldNames[0] == "counter");
//     REQUIRE(fieldNames[1] == "value");
// }

// TEST_CASE_METHOD(EncoderFixture, "Encoder - hasField check", "[encoder]") {
//     Encoder encoder(*schema_);

//     Packet packet(1, "SimplePacket");
//     packet.set("counter", uint32_t(1));
//     packet.set("value", int16_t(2));

//     REQUIRE(packet.hasField("counter") == true);
//     REQUIRE(packet.hasField("value") == true);
//     REQUIRE(packet.hasField("nonexistent") == false);
// }