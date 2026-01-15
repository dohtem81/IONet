#ifndef IONET_CORE_BYTE_BUFFER_H
#define IONET_CORE_BYTE_BUFFER_H

#include "Types.h"
#include "Endian.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include <stdexcept>

namespace ionet::core {

/// Writer for building binary buffers
class ByteBufferWriter {
public:
    ByteBufferWriter() = default;
    explicit ByteBufferWriter(std::size_t reserveSize);

    // Write methods
    void writeInt8(int8_t value);
    void writeInt16(int16_t value, ByteOrder order = ByteOrder::Native);
    void writeInt32(int32_t value, ByteOrder order = ByteOrder::Native);
    void writeInt64(int64_t value, ByteOrder order = ByteOrder::Native);

    void writeUInt8(uint8_t value);
    void writeUInt16(uint16_t value, ByteOrder order = ByteOrder::Native);
    void writeUInt32(uint32_t value, ByteOrder order = ByteOrder::Native);
    void writeUInt64(uint64_t value, ByteOrder order = ByteOrder::Native);

    void writeFloat32(float value, ByteOrder order = ByteOrder::Native);
    void writeFloat64(double value, ByteOrder order = ByteOrder::Native);

    void writeBytes(const uint8_t* data, std::size_t size);
    void writeBytes(const std::vector<uint8_t>& data);
    void writeString(const std::string& str, std::size_t fixedSize = 0);

    // Access
    const std::vector<uint8_t>& data() const { return buffer_; }
    std::vector<uint8_t> take() { return std::move(buffer_); }
    std::size_t size() const { return buffer_.size(); }
    void clear() { buffer_.clear(); }

private:
    std::vector<uint8_t> buffer_;
};

/// Reader for parsing binary buffers
class ByteBufferReader {
public:
    ByteBufferReader(const uint8_t* data, std::size_t size);
    explicit ByteBufferReader(const std::vector<uint8_t>& data);

    // Read methods
    int8_t readInt8();
    int16_t readInt16(ByteOrder order = ByteOrder::Native);
    int32_t readInt32(ByteOrder order = ByteOrder::Native);
    int64_t readInt64(ByteOrder order = ByteOrder::Native);

    uint8_t readUInt8();
    uint16_t readUInt16(ByteOrder order = ByteOrder::Native);
    uint32_t readUInt32(ByteOrder order = ByteOrder::Native);
    uint64_t readUInt64(ByteOrder order = ByteOrder::Native);

    float readFloat32(ByteOrder order = ByteOrder::Native);
    double readFloat64(ByteOrder order = ByteOrder::Native);

    std::vector<uint8_t> readBytes(std::size_t count);
    std::string readString(std::size_t size);

    // Position control
    std::size_t position() const { return pos_; }
    std::size_t remaining() const { return size_ - pos_; }
    std::size_t size() const { return size_; }
    bool atEnd() const { return pos_ >= size_; }

    void seek(std::size_t pos);
    void skip(std::size_t count);
    void reset() { pos_ = 0; }

private:
    const uint8_t* data_;
    std::size_t size_;
    std::size_t pos_ = 0;

    void checkRemaining(std::size_t needed) const;
};

} // namespace ionet::core

#endif