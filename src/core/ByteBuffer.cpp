#include "ionet/core/ByteBuffer.h"

namespace ionet::core {

// --- ByteBufferReader ---

ByteBufferReader::ByteBufferReader(std::span<const uint8_t> data, ByteOrder order)
    : data_(data), order_(order) {}

Result<std::span<const uint8_t>> ByteBufferReader::readBytes(std::size_t count) {
    alignToByte();
    
    if (bytePos_ + count > data_.size()) {
        return Error{
            "Buffer underflow: need " + std::to_string(count) +
            " bytes, have " + std::to_string(remaining()),
            bytePos_
        };
    }
    
    auto result = data_.subspan(bytePos_, count);
    bytePos_ += count;
    return result;
}

Result<std::vector<uint8_t>> ByteBufferReader::readBytesToVector(std::size_t count) {
    auto result = readBytes(count);
    if (result.hasError()) {
        return result.error();
    }
    
    auto span = result.value();
    return std::vector<uint8_t>(span.begin(), span.end());
}

Result<uint64_t> ByteBufferReader::readBits(std::size_t bitCount) {
    if (bitCount == 0 || bitCount > 64) {
        return Error{"Invalid bit count: " + std::to_string(bitCount), bytePos_};
    }
    
    uint64_t result = 0;
    std::size_t bitsRead = 0;
    
    while (bitsRead < bitCount) {
        if (bytePos_ >= data_.size()) {
            return Error{"Buffer underflow while reading bits", bytePos_};
        }
        
        std::size_t bitsAvailable = 8 - bitPos_;
        std::size_t bitsToRead = std::min(bitsAvailable, bitCount - bitsRead);
        
        uint8_t currentByte = data_[bytePos_];
        std::size_t shift = bitsAvailable - bitsToRead;
        uint8_t mask = ((1U << bitsToRead) - 1) << shift;
        uint8_t bits = (currentByte & mask) >> shift;
        
        result = (result << bitsToRead) | bits;
        bitsRead += bitsToRead;
        
        bitPos_ += bitsToRead;
        if (bitPos_ >= 8) {
            bitPos_ = 0;
            bytePos_++;
        }
    }
    
    return result;
}

void ByteBufferReader::alignToByte() {
    if (bitPos_ != 0) {
        bitPos_ = 0;
        bytePos_++;
    }
}

void ByteBufferReader::seek(std::size_t pos) {
    if (pos > data_.size()) {
        pos = data_.size();
    }
    bytePos_ = pos;
    bitPos_ = 0;
}

void ByteBufferReader::skip(std::size_t bytes) {
    alignToByte();
    seek(bytePos_ + bytes);
}

// --- ByteBufferWriter ---

ByteBufferWriter::ByteBufferWriter(ByteOrder order)
    : buffer_(ownedBuffer_), order_(order) {}

ByteBufferWriter::ByteBufferWriter(std::vector<uint8_t>& buffer, ByteOrder order)
    : buffer_(buffer), order_(order) {}

void ByteBufferWriter::writeBytes(std::span<const uint8_t> data) {
    flushBits();
    buffer_.insert(buffer_.end(), data.begin(), data.end());
}

void ByteBufferWriter::writeBits(uint64_t value, std::size_t bitCount) {
    if (bitCount == 0 || bitCount > 64) {
        return;
    }
    
    for (std::size_t i = bitCount; i > 0; --i) {
        uint8_t bit = (value >> (i - 1)) & 1;
        bitBuffer_ = (bitBuffer_ << 1) | bit;
        bitCount_++;
        
        if (bitCount_ == 8) {
            buffer_.push_back(bitBuffer_);
            bitBuffer_ = 0;
            bitCount_ = 0;
        }
    }
}

void ByteBufferWriter::flushBits() {
    if (bitCount_ > 0) {
        bitBuffer_ <<= (8 - bitCount_);
        buffer_.push_back(bitBuffer_);
        bitBuffer_ = 0;
        bitCount_ = 0;
    }
}

} // namespace ionet::core