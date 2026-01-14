#include <catch2/catch_test_macros.hpp>
#include <ionet/core/Endian.h>

using namespace ionet::core;

TEST_CASE("Byte swap 16-bit", "[endian]") {
    REQUIRE(endian::byteSwap<uint16_t>(0x1234) == 0x3412);
}

TEST_CASE("Byte swap 32-bit", "[endian]") {
    REQUIRE(endian::byteSwap<uint32_t>(0x12345678) == 0x78563412);
}

TEST_CASE("Byte swap 64-bit", "[endian]") {
    REQUIRE(endian::byteSwap<uint64_t>(0x0102030405060708ULL) == 0x0807060504030201ULL);
}

TEST_CASE("needsSwap logic", "[endian]") {
    REQUIRE(endian::needsSwap(ByteOrder::Native) == false);
}