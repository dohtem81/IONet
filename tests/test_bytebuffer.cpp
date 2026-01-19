#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <ionet/core/ByteBuffer.h>

using namespace ionet::core;


TEST_CASE("ByteBufferReader - readUInt16", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ByteBufferReader reader(data);
    REQUIRE(reader.readUInt16(ByteOrder::Big) == 0x0102);
    reader.seek(0);
    REQUIRE(reader.readUInt16(ByteOrder::Little) == 0x0201);
}

TEST_CASE("ByteBufferReader - readInt16", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ByteBufferReader reader(data);
    REQUIRE(reader.readInt16(ByteOrder::Big) == 0x0102);
    reader.seek(0);
    REQUIRE(reader.readInt16(ByteOrder::Little) == 0x0201);
}

TEST_CASE("ByteBufferReader - readUInt32", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ByteBufferReader reader(data);
    REQUIRE(reader.readUInt32(ByteOrder::Big) == 0x01020304);
    reader.seek(0);
    REQUIRE(reader.readUInt32(ByteOrder::Little) == 0x04030201);
}

TEST_CASE("ByteBufferReader - readInt32", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ByteBufferReader reader(data);
    REQUIRE(reader.readInt32(ByteOrder::Big) == 0x01020304);
    reader.seek(0);
    REQUIRE(reader.readInt32(ByteOrder::Little) == 0x04030201);
}

TEST_CASE("ByteBufferReader - readUInt64", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ByteBufferReader reader(data);
    REQUIRE(reader.readUInt64(ByteOrder::Big) == 0x0102030405060708ULL);
    reader.seek(0);
    REQUIRE(reader.readUInt64(ByteOrder::Little) == 0x0807060504030201ULL);
}

TEST_CASE("ByteBufferReader - readInt64", "[bytebuffer]") {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ByteBufferReader reader(data);
    REQUIRE(reader.readInt64(ByteOrder::Big) == 0x0102030405060708LL);
    reader.seek(0);
    REQUIRE(reader.readInt64(ByteOrder::Little) == 0x0807060504030201LL);
}

TEST_CASE("ByteBufferReader - readFloat32", "[bytebuffer]") {
    // 0x3F800000 is 1.0f in IEEE 754
    std::vector<uint8_t> float32_be = {0x3F, 0x80, 0x00, 0x00}; // 1.0f in big-endian
    ByteBufferReader float_reader(float32_be);
    REQUIRE(float_reader.readFloat32(ByteOrder::Big) == Catch::Approx(1.0f));
    float_reader.seek(0);
    // 0x0000803F as float (little-endian bytes of 1.0f)
    REQUIRE(float_reader.readFloat32(ByteOrder::Little) == Catch::Approx(4.600602988224807e-41f));
}

TEST_CASE("ByteBufferReader - readFloat64", "[bytebuffer]") {
    std::vector<uint8_t> float64_be = {0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 1.0 in big-endian
    ByteBufferReader double_reader(float64_be);
    REQUIRE(double_reader.readFloat64(ByteOrder::Big) == Catch::Approx(1.0));
    std::vector<uint8_t> float64_le = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f}; // 1.0 in little-endian
    ByteBufferReader double_reader_le(float64_le);
    REQUIRE(double_reader_le.readFloat64(ByteOrder::Little) == Catch::Approx(1)); //corrected
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


TEST_CASE("ByteBufferWriter - writeInt16", "[bytebuffer]") {
    ByteBufferWriter writer;
    writer.writeInt16(0x1234, ByteOrder::Big);
    auto data = writer.data();
    REQUIRE(data.size() == 2);
    REQUIRE(data[0] == 0x12);
    REQUIRE(data[1] == 0x34);

    ByteBufferWriter writer_le;
    writer_le.writeInt16(0x1234, ByteOrder::Little);
    auto data_le = writer_le.data();
    REQUIRE(data_le.size() == 2);
    REQUIRE(data_le[0] == 0x34);
    REQUIRE(data_le[1] == 0x12);
}

TEST_CASE("ByteBufferWriter - writeUInt16", "[bytebuffer]") {
    ByteBufferWriter writer;
    writer.writeUInt16(0xABCD, ByteOrder::Big);
    auto data = writer.data();
    REQUIRE(data.size() == 2);
    REQUIRE(data[0] == 0xAB);
    REQUIRE(data[1] == 0xCD);

    ByteBufferWriter writer_le;
    writer_le.writeUInt16(0xABCD, ByteOrder::Little);
    auto data_le = writer_le.data();
    REQUIRE(data_le.size() == 2);
    REQUIRE(data_le[0] == 0xCD);
    REQUIRE(data_le[1] == 0xAB);
}

TEST_CASE("ByteBufferWriter - writeInt32", "[bytebuffer]") {
    ByteBufferWriter writer;
    writer.writeInt32(0x12345678, ByteOrder::Big);
    auto data = writer.data();
    REQUIRE(data.size() == 4);
    REQUIRE(data[0] == 0x12);
    REQUIRE(data[1] == 0x34);
    REQUIRE(data[2] == 0x56);
    REQUIRE(data[3] == 0x78);

    ByteBufferWriter writer_le;
    writer_le.writeInt32(0x12345678, ByteOrder::Little);
    auto data_le = writer_le.data();
    REQUIRE(data_le.size() == 4);
    REQUIRE(data_le[0] == 0x78);
    REQUIRE(data_le[1] == 0x56);
    REQUIRE(data_le[2] == 0x34);
    REQUIRE(data_le[3] == 0x12);
}

TEST_CASE("ByteBufferWriter - writeUInt32", "[bytebuffer]") {
    ByteBufferWriter writer;
    writer.writeUInt32(0xDEADBEEF, ByteOrder::Big);
    auto data = writer.data();
    REQUIRE(data.size() == 4);
    REQUIRE(data[0] == 0xDE);
    REQUIRE(data[1] == 0xAD);
    REQUIRE(data[2] == 0xBE);
    REQUIRE(data[3] == 0xEF);

    ByteBufferWriter writer_le;
    writer_le.writeUInt32(0xDEADBEEF, ByteOrder::Little);
    auto data_le = writer_le.data();
    REQUIRE(data_le.size() == 4);
    REQUIRE(data_le[0] == 0xEF);
    REQUIRE(data_le[1] == 0xBE);
    REQUIRE(data_le[2] == 0xAD);
    REQUIRE(data_le[3] == 0xDE);
}

TEST_CASE("ByteBufferWriter - writeInt64", "[bytebuffer]") {
    ByteBufferWriter writer;
    writer.writeInt64(0x1122334455667788LL, ByteOrder::Big);
    auto data = writer.data();
    REQUIRE(data.size() == 8);
    REQUIRE(data[0] == 0x11);
    REQUIRE(data[1] == 0x22);
    REQUIRE(data[2] == 0x33);
    REQUIRE(data[3] == 0x44);
    REQUIRE(data[4] == 0x55);
    REQUIRE(data[5] == 0x66);
    REQUIRE(data[6] == 0x77);
    REQUIRE(data[7] == 0x88);

    ByteBufferWriter writer_le;
    writer_le.writeInt64(0x1122334455667788LL, ByteOrder::Little);
    auto data_le = writer_le.data();
    REQUIRE(data_le.size() == 8);
    REQUIRE(data_le[0] == 0x88);
    REQUIRE(data_le[1] == 0x77);
    REQUIRE(data_le[2] == 0x66);
    REQUIRE(data_le[3] == 0x55);
    REQUIRE(data_le[4] == 0x44);
    REQUIRE(data_le[5] == 0x33);
    REQUIRE(data_le[6] == 0x22);
    REQUIRE(data_le[7] == 0x11);
}

TEST_CASE("ByteBufferWriter - writeUInt64", "[bytebuffer]") {
    ByteBufferWriter writer;
    writer.writeUInt64(0xCAFEBABEDEADBEEF, ByteOrder::Big);
    auto data = writer.data();
    REQUIRE(data.size() == 8);
    REQUIRE(data[0] == 0xCA);
    REQUIRE(data[1] == 0xFE);
    REQUIRE(data[2] == 0xBA);
    REQUIRE(data[3] == 0xBE);
    REQUIRE(data[4] == 0xDE);
    REQUIRE(data[5] == 0xAD);
    REQUIRE(data[6] == 0xBE);
    REQUIRE(data[7] == 0xEF);

    ByteBufferWriter writer_le;
    writer_le.writeUInt64(0xCAFEBABEDEADBEEF, ByteOrder::Little);
    auto data_le = writer_le.data();
    REQUIRE(data_le.size() == 8);
    REQUIRE(data_le[0] == 0xEF);
    REQUIRE(data_le[1] == 0xBE);
    REQUIRE(data_le[2] == 0xAD);
    REQUIRE(data_le[3] == 0xDE);
    REQUIRE(data_le[4] == 0xBE);
    REQUIRE(data_le[5] == 0xBA);
    REQUIRE(data_le[6] == 0xFE);
    REQUIRE(data_le[7] == 0xCA);
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