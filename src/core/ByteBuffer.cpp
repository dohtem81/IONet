#include "ionet/core/ByteBuffer.h"

namespace ionet::core {

uint64_t ByteBuffer::readBits(std::size_t bitCount) {
    if (bitCount == 0 || bitCount > 64) {
        throw std::invalid_argument("bitCount must be between 1 and 64");
    }

    // Sync bit position with byte position if we've done byte reads since last bit read
    if (lastOpWasByteRead_) {
        bitPosition_ = position_ * 8;
        lastOpWasByteRead_ = false;
    }

    uint64_t result = 0;
    for (std::size_t i = 0; i < bitCount; ++i) {
        if (bitPosition_ / 8 >= data_.size()) {
            throw std::out_of_range("Buffer underflow while reading bits");
        }

        std::size_t byteIndex = bitPosition_ / 8;
        std::size_t bitIndex = 7 - (bitPosition_ % 8); // Read from MSB to LSB

        uint8_t currentByte = data_[byteIndex];
        uint8_t bit = (currentByte >> bitIndex) & 0x01;

        result = (result << 1) | bit;
        ++bitPosition_;
    }

    // Update position_ if we've crossed byte boundaries
    position_ = (bitPosition_ + 7) / 8;

    return result;
}



} // namespace ionet::core