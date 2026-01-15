#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <ionet/core/ByteBuffer.h>

using namespace ionet::core;

TEST_CASE("ByteBufferReader - read integers big endian", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ByteBufferReader reader(data);

    auto val16 = reader.readUInt16(ByteOrder::Big);
    REQUIRE(val16 == 0x0102);
}

TEST_CASE("ByteBufferReader - buffer underflow", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02};
    ByteBufferReader reader(data);

    // Try to read 4 bytes as uint32_t, should throw
    try {
        reader.readUInt32(ByteOrder::Big);
        FAIL("Expected exception not thrown");
    } catch (const std::exception&) {
        SUCCEED();
    }
}

TEST_CASE("ByteBufferWriter - write integers", "[bytebuffer]") {
    ByteBufferWriter writer;

    writer.writeUInt16(0x0102, ByteOrder::Big);

    auto data = writer.data();
    REQUIRE(data.size() == 2);
    REQUIRE(data[0] == 0x01);
    REQUIRE(data[1] == 0x02);
}

TEST_CASE("Round trip read/write", "[bytebuffer]") {
    ByteBufferWriter writer;
    writer.writeFloat32(3.14159f, ByteOrder::Big);
    writer.writeInt32(-42, ByteOrder::Big);

    ByteBufferReader reader(writer.data());

    float f = reader.readFloat32(ByteOrder::Big);
    int32_t i = reader.readInt32(ByteOrder::Big);

    REQUIRE(f == Catch::Approx(3.14159f));
    REQUIRE(i == -42);
}