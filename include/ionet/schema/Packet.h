#ifndef IONET_SCHEMA_PACKET_H
#define IONET_SCHEMA_PACKET_H

#include "Field.h"
#include <vector>
#include <string>
#include <optional>
#include <algorithm>
#include <yaml-cpp/yaml.h>
#include <variant>
#include <unordered_map>

namespace ionet::schema {

/// Definition of a packet structure
struct Packet {
    uint32_t id = 0;                      // Packet identifier
    std::string name;
    std::string description;
    std::vector<Field> fields;

    /// constructor
    Packet() = default;  // Add default constructor
    Packet(uint32_t packetId, const std::string& packetName)
        : id(packetId), name(packetName) {}
    
    /// Calculate total byte size of packet
    std::size_t totalSize() const {
        std::size_t size = 0;
        for (const auto& field : fields) {
            size += field.byteSize();
        }
        return size;
    }
    
    /// Check if all fields have fixed size
    bool isFixedSize() const {
        return std::all_of(fields.begin(), fields.end(),
            [](const Field& f) { return f.isFixedSize(); });
    }
    
    /// Find field by name
    const Field* findField(const std::string& name) const {
        auto it = std::find_if(fields.begin(), fields.end(),
            [&name](const Field& f) { return f.name == name; });
        return it != fields.end() ? &(*it) : nullptr;
    }
    
    /// Get field index by name, returns -1 if not found
    int fieldIndex(const std::string& name) const {
        for (std::size_t i = 0; i < fields.size(); ++i) {
            if (fields[i].name == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    /// hasField function to check existence of a field
    bool hasField(const std::string& field) const {
        return fieldIndex(field) != -1;
    }

    template<typename T>
    void set(const std::string& field, const T& value) {
        if (hasField(field)) {
            const Field* f = findField(field);
            if constexpr (std::is_floating_point_v<T>) {
                if (f && f->scaling) {
                    // For scaled fields, raw = (value - offset) / scale
                    double raw = (static_cast<double>(value) - f->scaling.value().offset) / f->scaling.value().scale;
                    // Set as int64_t for signed types, uint64_t for unsigned
                    if (f->type == core::DataType::Int8 || f->type == core::DataType::Int16 ||
                        f->type == core::DataType::Int32 || f->type == core::DataType::Int64) {
                        data_[field] = static_cast<int64_t>(raw);
                    } else {
                        data_[field] = static_cast<uint64_t>(raw);
                    }
                    return;
                }
            }
        }
        // Normal storage: match 'add' logic
        if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
            data_[field] = static_cast<int64_t>(value);
        } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            data_[field] = static_cast<uint64_t>(value);
        } else if constexpr (std::is_floating_point_v<T>) {
            data_[field] = static_cast<double>(value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            data_[field] = value;
        } else {
            throw std::runtime_error("Unsupported type for field: " + field);
        }
    }

    const std::variant<uint64_t, int64_t, double, std::string>& rawValue(const std::string& field) const {
        auto it = data_.find(field);
        if (it == data_.end()) {
            throw std::runtime_error("Field not found: " + field);
        }
        return it->second;
    }

    /// define new field and add it, recalculate total size
    template<typename T>
    void add(const std::string& field, const T& value) {
        if (hasField(field)) {
            throw std::runtime_error("Field already exists: " + field);
        } else {
            fields.push_back(Field{field});
        }
        // Cast to match variant types
        if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            data_[field] = static_cast<uint64_t>(value);
        } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
            data_[field] = static_cast<int64_t>(value);
        } else if constexpr (std::is_floating_point_v<T>) {
            data_[field] = static_cast<double>(value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            data_[field] = value;
        } else {
            throw std::runtime_error("Unsupported type for field: " + field);
        }
    }

    /// list fields (from set data)
    std::vector<std::string> listFields() const {
        std::vector<std::string> fieldNames;
        for (const auto& pair : data_) {
            fieldNames.push_back(pair.first);
        }
        return fieldNames;
    }

private:
    std::unordered_map<std::string, std::variant<uint64_t, int64_t, double, std::string>> data_;
};

} // namespace ionet::schema

#endif