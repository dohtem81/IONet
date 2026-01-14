#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "ionet/codec/Decoder.h"
#include "ionet/schema/SchemaBuilder.h"
#include <vector>

using namespace ionet;
using namespace ionet::codec;
using namespace ionet::schema;

TEST_CASE("Decoder - Basic integer decoding", "[decoder]") {
    // Build a simple schema
    Schema schema = SchemaBuilder()
        .name("TestSchema")
        .bigEndian()
        .packet(0x01, "IntPacket")
            .field("value_u8", core::DataType::UInt8)
            .field("value_u16", core::DataType::UInt16)
            .field("value_u32", core::DataType::UInt32)
        .build();
    
    Decoder decoder(schema);
    
    // Create test data: u8=42, u16=1000, u32=100000
    std::vector<uint8_t> data = {
        42,                          // u8
        0x03, 0xE8,                 // u16 = 1000
        0x00, 0x01, 0x86, 0xA0      // u32 = 100000
    };
    
    auto result = decoder.decode(0x01, data);
    
    REQUIRE(result.has_value());
    
    const auto& packet = result.value();
    REQUIRE(packet.packetName == "IntPacket");
    REQUIRE(packet.fieldCount() == 3);
    
    REQUIRE(std::get<uint64_t>(packet["value_u8"]) == 42);
    REQUIRE(std::get<uint64_t>(packet["value_u16"]) == 1000);
    REQUIRE(std::get<uint64_t>(packet["value_u32"]) == 100000);
}

TEST_CASE("Decoder - Signed integers", "[decoder]") {
    Schema schema = SchemaBuilder()
        .name("TestSchema")
        .bigEndian()
        .packet(0x02, "SignedPacket")
            .field("value_i8", core::DataType::Int8)
            .field("value_i16", core::DataType::Int16)
            .field("value_i32", core::DataType::Int32)
        .build();
    
    Decoder decoder(schema);
    
    // Create test data: i8=-10, i16=-1000, i32=-100000
    std::vector<uint8_t> data = {
        0xF6,                       // i8 = -10
        0xFC, 0x18,                // i16 = -1000
        0xFF, 0xFE, 0x79, 0x60     // i32 = -100000
    };
    
    auto result = decoder.decode(0x02, data);
    
    REQUIRE(result.has_value());
    
    const auto& packet = result.value();
    REQUIRE(std::get<int64_t>(packet["value_i8"]) == -10);
    REQUIRE(std::get<int64_t>(packet["value_i16"]) == -1000);
    REQUIRE(std::get<int64_t>(packet["value_i32"]) == -100000);
}

TEST_CASE("Decoder - Floating point", "[decoder]") {
    Schema schema = SchemaBuilder()
        .name("TestSchema")
        .bigEndian()
        .packet(0x03, "FloatPacket")
            .field("temp", core::DataType::Float32)
            .field("pressure", core::DataType::Float64)
        .build();
    
    Decoder decoder(schema);
    
    // Create test data (big endian floats)
    std::vector<uint8_t> data = {
        0x42, 0x28, 0x00, 0x00,     // float32 = 42.0
        0x40, 0x09, 0x21, 0xFB, 0x54, 0x44, 0x2D, 0x18  // float64 = 3.14159
    };
    
    auto result = decoder.decode(0x03, data);
    
    REQUIRE(result.has_value());
    
    const auto& packet = result.value();
    REQUIRE(std::get<double>(packet["temp"]) == Catch::Approx(42.0));
    REQUIRE(std::get<double>(packet["pressure"]) == Catch::Approx(3.14159));
}

TEST_CASE("Decoder - Integer scaling", "[decoder]") {
    Schema schema = SchemaBuilder()
        .name("TestSchema")
        .bigEndian()
        .packet(0x04, "ScaledPacket")
            .field("temperature", core::DataType::Int16)
                .scale(0.01)
                .offset(-40.0)
            .field("voltage", core::DataType::UInt16)
                .scale(0.001)
            .done()
        .build();
    
    Decoder decoder(schema);
    
    // Raw: i16=5000 -> (5000 * 0.01) - 40 = 10.0°C
    // Raw: u16=3300 -> 3300 * 0.001 = 3.3V
    std::vector<uint8_t> data = {
        0x13, 0x88,                 // 5000
        0x0C, 0xE4                  // 3300
    };
    
    auto result = decoder.decode(0x04, data);
    
    REQUIRE(result.has_value());
    
    const auto& packet = result.value();
    REQUIRE(std::get<double>(packet["temperature"]) == Catch::Approx(10.0));
    REQUIRE(std::get<double>(packet["voltage"]) == Catch::Approx(3.3));
}

TEST_CASE("Decoder - Bitfield with flags", "[decoder]") {
    Schema schema = SchemaBuilder()
        .name("TestSchema")
        .bigEndian()
        .packet(0x05, "StatusPacket")
            .bitfield("status", 8)
                .flag(0, "engine_1")
                .flag(1, "engine_2")
                .flag(7, "abort")
            .done()
        .build();
    
    Decoder decoder(schema);
    
    // Bits: 10000011 = engines 1,2 on, abort on
    std::vector<uint8_t> data = { 0x83 };
    
    auto result = decoder.decode(0x05, data);
    
    REQUIRE(result.has_value());
    
    const auto& packet = result.value();
    const auto& flags = std::get<BitfieldValue>(packet["status"]);
    
    REQUIRE(flags.at("engine_1") == true);
    REQUIRE(flags.at("engine_2") == true);
    REQUIRE(flags.at("abort") == true);
}

TEST_CASE("Decoder - String field", "[decoder]") {
    Schema schema = SchemaBuilder()
        .name("TestSchema")
        .packet(0x06, "StringPacket")
            .string("name", 16)
        .build();
    
    Decoder decoder(schema);
    
    std::vector<uint8_t> data = {
        'H', 'e', 'l', 'l', 'o', 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    
    auto result = decoder.decode(0x06, data);
    
    REQUIRE(result.has_value());
    
    const auto& packet = result.value();
    REQUIRE(std::get<std::string>(packet["name"]) == "Hello");
}

TEST_CASE("Decoder - Little endian", "[decoder]") {
    Schema schema = SchemaBuilder()
        .name("TestSchema")
        .littleEndian()
        .packet(0x07, "LittleEndianPacket")
            .field("value", core::DataType::UInt32)
        .build();
    
    Decoder decoder(schema);
    
    // Little endian: 0x12345678 stored as 78 56 34 12
    std::vector<uint8_t> data = { 0x78, 0x56, 0x34, 0x12 };
    
    auto result = decoder.decode(0x07, data);
    
    REQUIRE(result.has_value());
    REQUIRE(std::get<uint64_t>(result.value()["value"]) == 0x12345678);
}

TEST_CASE("Decoder - Error handling", "[decoder]") {
    Schema schema = SchemaBuilder()
        .name("TestSchema")
        .packet(0x08, "TestPacket")
            .field("value", core::DataType::UInt32)
        .build();
    
    Decoder decoder(schema);
    
    SECTION("Unknown packet ID") {
        std::vector<uint8_t> data = { 0x01, 0x02, 0x03, 0x04 };
        auto result = decoder.decode(0x99, data);
        
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().message.find("Unknown packet ID") != std::string::npos);
    }
    
    SECTION("Insufficient data") {
        std::vector<uint8_t> data = { 0x01, 0x02 };  // Only 2 bytes, need 4
        auto result = decoder.decode(0x08, data);
        
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().message.find("Insufficient data") != std::string::npos);
    }
    
    SECTION("Buffer underflow during read") {
        std::vector<uint8_t> data = { 0x01, 0x02, 0x03 };  // Only 3 bytes
        auto result = decoder.decode(0x08, data);
        
        REQUIRE_FALSE(result.has_value());
    }
}

TEST_CASE("Decoder - Complex packet", "[decoder]") {
    Schema schema = SchemaBuilder()
        .name("RocketTelemetry")
        .bigEndian()
        .packet(0x01, "FlightData")
            .field("timestamp", core::DataType::UInt64)
            .field("altitude", core::DataType::Float32)
            .field("velocity", core::DataType::Float32)
            .bitfield("status", 8)
                .flag(0, "engine_on")
                .flag(1, "parachute_deployed")
            .field("temperature", core::DataType::Int16)
                .scale(0.1)
            .done()
        .build();
    
    Decoder decoder(schema);
    
    std::vector<uint8_t> data = {
        // timestamp = 123456789
        0x00, 0x00, 0x00, 0x00, 0x07, 0x5B, 0xCD, 0x15,
        // altitude = 1000.25 (0x447A1000 in big-endian float32)
        0x44, 0x7A, 0x10, 0x00,
        // velocity = 50.25
        0x42, 0x49, 0x00, 0x00,
        // status = 0x01 (engine on)
        0x01,
        // temperature = 250 -> 25.0°C
        0x00, 0xFA
    };
    
    auto result = decoder.decode(0x01, data);
    
    REQUIRE(result.has_value());
    
    const auto& packet = result.value();
    REQUIRE(packet.packetName == "FlightData");
    
    REQUIRE(std::get<uint64_t>(packet["timestamp"]) == 123456789);
    REQUIRE(std::get<double>(packet["altitude"]) == Catch::Approx(1000.25));
    REQUIRE(std::get<double>(packet["velocity"]) == Catch::Approx(50.25));
    
    const auto& status = std::get<BitfieldValue>(packet["status"]);
    REQUIRE(status.at("engine_on") == true);
    REQUIRE(status.at("parachute_deployed") == false);
    
    REQUIRE(std::get<double>(packet["temperature"]) == Catch::Approx(25.0));
}
