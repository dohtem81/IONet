#include <catch2/catch_test_macros.hpp>
#include <ionet/core/ByteBuffer.h>

using namespace ionet::core;

TEST_CASE("ByteBufferReader - read integers big endian", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ByteBufferReader reader(data, ByteOrder::Big);
    
    auto val16 = reader.read<uint16_t>();
    REQUIRE(val16.ok());
    REQUIRE(val16.value() == 0x0102);
}

TEST_CASE("ByteBufferReader - buffer underflow", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02};
    ByteBufferReader reader(data, ByteOrder::Big);
    
    auto result = reader.read<uint32_t>();
    REQUIRE(result.hasError());
}

TEST_CASE("ByteBufferWriter - write integers", "[bytebuffer]") {
    ByteBufferWriter writer(ByteOrder::Big);
    
    writer.write<uint16_t>(0x0102);
    
    auto data = writer.data();
    REQUIRE(data.size() == 2);
    REQUIRE(data[0] == 0x01);
    REQUIRE(data[1] == 0x02);
}

TEST_CASE("Round trip read/write", "[bytebuffer]") {
    ByteBufferWriter writer(ByteOrder::Big);
    writer.write<float>(3.14159f);
    writer.write<int32_t>(-42);
    
    ByteBufferReader reader(writer.data(), ByteOrder::Big);
    
    REQUIRE(reader.read<float>().value() == 3.14159f);
    REQUIRE(reader.read<int32_t>().value() == -42);
}