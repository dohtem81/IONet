#ifndef IONET_CORE_RESULT_H
#define IONET_CORE_RESULT_H

#include <variant>
#include <string>
#include <stdexcept>

namespace ionet::core {

struct Error {
    std::string message;
    std::size_t position = 0;
    
    Error(std::string msg, std::size_t pos = 0)
        : message(std::move(msg)), position(pos) {}
};

template<typename T>
class Result {
public:
    Result(T value) : data_(std::move(value)) {}
    Result(Error error) : data_(std::move(error)) {}
    
    bool ok() const { return std::holds_alternative<T>(data_); }
    bool hasError() const { return std::holds_alternative<Error>(data_); }
    
    T& value() & {
        if (hasError()) {
            throw std::runtime_error(error().message);
        }
        return std::get<T>(data_);
    }
    
    const T& value() const& {
        if (hasError()) {
            throw std::runtime_error(error().message);
        }
        return std::get<T>(data_);
    }
    
    T&& value() && {
        if (hasError()) {
            throw std::runtime_error(error().message);
        }
        return std::move(std::get<T>(data_));
    }
    
    const Error& error() const {
        if (ok()) {
            throw std::logic_error("Result is not an error");
        }
        return std::get<Error>(data_);
    }
    
    T valueOr(T defaultValue) const {
        if (ok()) {
            return std::get<T>(data_);
        }
        return defaultValue;
    }

private:
    std::variant<T, Error> data_;
};

} // namespace ionet::core

#endif