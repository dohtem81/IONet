#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <ionet/schema/Schema.h>
#include <ionet/schema/SchemaBuilder.h>

using namespace ionet::schema;
using namespace ionet::core;

TEST_CASE("Field - byte size calculation", "[schema]") {
    Field f;
    
    f.type = DataType::UInt8;
    REQUIRE(f.byteSize() == 1);
    
    f.type = DataType::UInt32;
    REQUIRE(f.byteSize() == 4);
}

TEST_CASE("Field - scaling", "[schema]") {
    Scaling s{0.01, -40.0};
    REQUIRE_THAT(s.apply(6000), Catch::Matchers::WithinAbs(20.0, 0.001));
}

TEST_CASE("Packet - total size", "[schema]") {
    Packet p;
    p.fields.push_back(Field{.name = "a", .type = DataType::UInt8});
    p.fields.push_back(Field{.name = "b", .type = DataType::UInt16});
    p.fields.push_back(Field{.name = "c", .type = DataType::UInt32});
    
    REQUIRE(p.totalSize() == 7);
}

TEST_CASE("Schema - packet lookup", "[schema]") {
    Schema schema;
    
    Packet p1;
    p1.id = 0x01;
    p1.name = "Telemetry";
    p1.fields.push_back(Field{.name = "data", .type = DataType::UInt8});
    
    schema.addPacket(p1);
    
    REQUIRE(schema.packetCount() == 1);
    REQUIRE(schema.findPacketById(0x01) != nullptr);
    REQUIRE(schema.findPacketById(0x01)->name == "Telemetry");
}

TEST_CASE("SchemaBuilder - fluent API", "[schema]") {
    auto schema = SchemaBuilder()
        .name("RocketTelemetry")
        .version("1.0")
        .bigEndian()
        .packet(0x01, "FlightData")
            .uint64("timestamp").unit("microseconds")
            .float32("altitude").unit("meters")
        .build();
    
    REQUIRE(schema.info().name == "RocketTelemetry");
    REQUIRE(schema.packetCount() == 1);
}