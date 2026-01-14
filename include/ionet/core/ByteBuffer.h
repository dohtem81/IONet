#ifndef IONET_BYTEBUFFER_H
#define IONET_BYTEBUFFER_H

#include "Types.h"
#include "Endian.h"
#include <span>
#include <stdexcept>
#include <cstring>

namespace ionet::core {

class ByteBuffer {
public:
    // Non-owning view of data
    explicit ByteBuffer(std::span<const uint8_t> data, ByteOrder order = ByteOrder::Big)
        : data_(data), order_(order), position_(0) {}

    // Read with automatic endian conversion
    template<typename T>
    T read() {
        if (position_ + sizeof(T) > data_.size()) {
            throw std::out_of_range("Buffer underflow");
        }
        
        T value;
        std::memcpy(&value, data_.data() + position_, sizeof(T));
        position_ += sizeof(T);
        
        return endian::convert<T>(value, order_);
    }

    // Read raw bytes
    std::span<const uint8_t> readBytes(std::size_t count) {
        if (position_ + count > data_.size()) {
            throw std::out_of_range("Buffer underflow");
        }
        auto result = data_.subspan(position_, count);
        position_ += count;
        return result;
    }

    // Read bits from current byte
    uint64_t readBits(std::size_t bitCount);

    std::size_t position() const { return position_; }
    std::size_t remaining() const { return data_.size() - position_; }
    void seek(std::size_t pos) { position_ = pos; }

private:
    std::span<const uint8_t> data_;
    ByteOrder order_;
    std::size_t position_;
    
    // For bit-level reading
    std::size_t bitPosition_ = 0;
};

} // namespace ionet::core

#endif