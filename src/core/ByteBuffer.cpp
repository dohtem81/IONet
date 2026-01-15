#include "../../include/ionet/core/ByteBuffer.h"

namespace ionet::core {

// ============ ByteBufferWriter ============

ByteBufferWriter::ByteBufferWriter(std::size_t reserveSize) {
    buffer_.reserve(reserveSize);
}

void ByteBufferWriter::writeInt8(int8_t value) {
    buffer_.push_back(static_cast<uint8_t>(value));
}

void ByteBufferWriter::writeInt16(int16_t value, ByteOrder order) {
    uint16_t ordered = endian::convert(static_cast<uint16_t>(value), order);
    buffer_.push_back(static_cast<uint8_t>(ordered >> 8));
    buffer_.push_back(static_cast<uint8_t>(ordered & 0xFF));
}

void ByteBufferWriter::writeInt32(int32_t value, ByteOrder order) {
    uint32_t ordered = endian::convert(static_cast<uint32_t>(value), order);
    buffer_.push_back(static_cast<uint8_t>(ordered >> 24));
    buffer_.push_back(static_cast<uint8_t>(ordered >> 16));
    buffer_.push_back(static_cast<uint8_t>(ordered >> 8));
    buffer_.push_back(static_cast<uint8_t>(ordered & 0xFF));
}

void ByteBufferWriter::writeInt64(int64_t value, ByteOrder order) {
    uint64_t ordered = endian::convert(static_cast<uint64_t>(value), order);
    for (int i = 7; i >= 0; --i) {
        buffer_.push_back(static_cast<uint8_t>(ordered >> (i * 8)));
    }
}

void ByteBufferWriter::writeUInt8(uint8_t value) {
    buffer_.push_back(value);
}

void ByteBufferWriter::writeUInt16(uint16_t value, ByteOrder order) {
    uint16_t ordered = endian::convert(value, order);
    buffer_.push_back(static_cast<uint8_t>(ordered >> 8));
    buffer_.push_back(static_cast<uint8_t>(ordered & 0xFF));
}

void ByteBufferWriter::writeUInt32(uint32_t value, ByteOrder order) {
    uint32_t ordered = endian::convert(value, order);
    buffer_.push_back(static_cast<uint8_t>(ordered >> 24));
    buffer_.push_back(static_cast<uint8_t>(ordered >> 16));
    buffer_.push_back(static_cast<uint8_t>(ordered >> 8));
    buffer_.push_back(static_cast<uint8_t>(ordered & 0xFF));
}

void ByteBufferWriter::writeUInt64(uint64_t value, ByteOrder order) {
    uint64_t ordered = endian::convert(value, order);
    for (int i = 7; i >= 0; --i) {
        buffer_.push_back(static_cast<uint8_t>(ordered >> (i * 8)));
    }
}

void ByteBufferWriter::writeFloat32(float value, ByteOrder order) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    writeUInt32(bits, order);
}

void ByteBufferWriter::writeFloat64(double value, ByteOrder order) {
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    writeUInt64(bits, order);
}

void ByteBufferWriter::writeBytes(const uint8_t* data, std::size_t size) {
    buffer_.insert(buffer_.end(), data, data + size);
}

void ByteBufferWriter::writeBytes(const std::vector<uint8_t>& data) {
    buffer_.insert(buffer_.end(), data.begin(), data.end());
}

void ByteBufferWriter::writeString(const std::string& str, std::size_t fixedSize) {
    if (fixedSize == 0) {
        buffer_.insert(buffer_.end(), str.begin(), str.end());
    } else {
        std::size_t copySize = std::min(str.size(), fixedSize);
        buffer_.insert(buffer_.end(), str.begin(), str.begin() + copySize);
        // Pad with zeros
        for (std::size_t i = copySize; i < fixedSize; ++i) {
            buffer_.push_back(0);
        }
    }
}

// ============ ByteBufferReader ============

ByteBufferReader::ByteBufferReader(const uint8_t* data, std::size_t size)
    : data_(data), size_(size), pos_(0) {}

ByteBufferReader::ByteBufferReader(const std::vector<uint8_t>& data)
    : data_(data.data()), size_(data.size()), pos_(0) {}

void ByteBufferReader::checkRemaining(std::size_t needed) const {
    if (pos_ + needed > size_) {
        throw std::runtime_error("Buffer underflow: need " + 
            std::to_string(needed) + " bytes, have " + 
            std::to_string(remaining()));
    }
}

int8_t ByteBufferReader::readInt8() {
    checkRemaining(1);
    return static_cast<int8_t>(data_[pos_++]);
}

int16_t ByteBufferReader::readInt16(ByteOrder order) {
    checkRemaining(2);
    uint16_t value = (static_cast<uint16_t>(data_[pos_]) << 8) |
                      static_cast<uint16_t>(data_[pos_ + 1]);
    pos_ += 2;
    return static_cast<int16_t>(endian::convert(value, order));
}

int32_t ByteBufferReader::readInt32(ByteOrder order) {
    checkRemaining(4);
    uint32_t value = (static_cast<uint32_t>(data_[pos_]) << 24) |
                     (static_cast<uint32_t>(data_[pos_ + 1]) << 16) |
                     (static_cast<uint32_t>(data_[pos_ + 2]) << 8) |
                      static_cast<uint32_t>(data_[pos_ + 3]);
    pos_ += 4;
    return static_cast<int32_t>(endian::convert(value, order));
}

int64_t ByteBufferReader::readInt64(ByteOrder order) {
    checkRemaining(8);
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value = (value << 8) | static_cast<uint64_t>(data_[pos_ + i]);
    }
    pos_ += 8;
    return static_cast<int64_t>(endian::convert(value, order));
}

uint8_t ByteBufferReader::readUInt8() {
    checkRemaining(1);
    return data_[pos_++];
}

uint16_t ByteBufferReader::readUInt16(ByteOrder order) {
    checkRemaining(2);
    uint16_t value = (static_cast<uint16_t>(data_[pos_]) << 8) |
                      static_cast<uint16_t>(data_[pos_ + 1]);
    pos_ += 2;
    return endian::convert(value, order);
}

uint32_t ByteBufferReader::readUInt32(ByteOrder order) {
    checkRemaining(4);
    uint32_t value = (static_cast<uint32_t>(data_[pos_]) << 24) |
                     (static_cast<uint32_t>(data_[pos_ + 1]) << 16) |
                     (static_cast<uint32_t>(data_[pos_ + 2]) << 8) |
                      static_cast<uint32_t>(data_[pos_ + 3]);
    pos_ += 4;
    return endian::convert(value, order);
}

uint64_t ByteBufferReader::readUInt64(ByteOrder order) {
    checkRemaining(8);
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value = (value << 8) | static_cast<uint64_t>(data_[pos_ + i]);
    }
    pos_ += 8;
    return endian::convert(value, order);
}

float ByteBufferReader::readFloat32(ByteOrder order) {
    uint32_t bits = readUInt32(order);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

double ByteBufferReader::readFloat64(ByteOrder order) {
    uint64_t bits = readUInt64(order);
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::vector<uint8_t> ByteBufferReader::readBytes(std::size_t count) {
    checkRemaining(count);
    std::vector<uint8_t> result(data_ + pos_, data_ + pos_ + count);
    pos_ += count;
    return result;
}

std::string ByteBufferReader::readString(std::size_t size) {
    checkRemaining(size);
    std::string result(reinterpret_cast<const char*>(data_ + pos_), size);
    pos_ += size;
    return result;
}

void ByteBufferReader::seek(std::size_t pos) {
    if (pos > size_) {
        throw std::runtime_error("Seek past end of buffer");
    }
    pos_ = pos;
}

void ByteBufferReader::skip(std::size_t count) {
    checkRemaining(count);
    pos_ += count;
}

} // namespace ionet::core