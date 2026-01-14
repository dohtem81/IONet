#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "ionet/codec/Decoder.h"
#include "ionet/schema/SchemaBuilder.h"
#include <iostream>
#include <vector>

using namespace ionet;
using namespace ionet::codec;
using namespace ionet::schema;

TEST_CASE("Debug scaling", "[debug]") {
    Schema schema = SchemaBuilder()
        .name("TestSchema")
        .bigEndian()
        .packet(0x04, "ScaledPacket")
            .field("temperature", core::DataType::Int16)
                .scale(0.01)
                .offset(-40.0)
            .done()
        .build();
    
    // Check the schema
    const auto* packet = schema.findPacketById(0x04);
    REQUIRE(packet != nullptr);
    REQUIRE(packet->fields.size() == 1);
    
    const auto& field = packet->fields[0];
    std::cout << "Field name: " << field.name << std::endl;
    std::cout << "Has scaling: " << field.hasScaling() << std::endl;
    if (field.scaling) {
        std::cout << "Scale: " << field.scaling->scale << std::endl;
        std::cout << "Offset: " << field.scaling->offset << std::endl;
    }
    
    // Raw: i16=5000 -> (5000 * 0.01) - 40 = 10.0°C
    std::vector<uint8_t> data = {
        0x13, 0x88                  // 5000
    };
    
    // Test direct buffer read
    core::ByteBuffer buf(data, core::ByteOrder::Big);
    int16_t rawVal = buf.read<int16_t>();
    std::cout << "Raw int16 value: " << rawVal << std::endl;
    std::cout << "Expected: 5000" << std::endl;
    
    Decoder decoder(schema);
    auto result = decoder.decode(0x04, data);
    
    REQUIRE(result.has_value());
    
    const auto& decoded = result.value();
    std::cout << "Temperature value: " << std::get<double>(decoded["temperature"]) << std::endl;
}
