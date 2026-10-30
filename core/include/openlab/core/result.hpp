#pragma once

#include <string>
#include <utility>
#include <variant>

namespace openlab::core {

enum class ErrorCode {
    InvalidArgument,
    NotFound,
    Conflict,
    Incompatible,
    Io,
    CorruptData,
    UnsupportedVersion,
    ResourceLimit
};

struct Error {
    ErrorCode code;
    std::string message;
};

template <typename T>
class Result {
public:
    [[nodiscard]] static Result success(T value) { return Result(std::move(value)); }
    [[nodiscard]] static Result failure(Error error) { return Result(std::move(error)); }

    [[nodiscard]] explicit operator bool() const noexcept { return std::holds_alternative<T>(data_); }
    [[nodiscard]] const T& value() const& { return std::get<T>(data_); }
    [[nodiscard]] T&& value() && { return std::get<T>(std::move(data_)); }
    [[nodiscard]] const Error& error() const& { return std::get<Error>(data_); }

private:
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(Error error) : data_(std::move(error)) {}
    std::variant<T, Error> data_;
};

template <>
class Result<void> {
public:
    [[nodiscard]] static Result success() { return Result(); }
    [[nodiscard]] static Result failure(Error error) { return Result(std::move(error)); }

    [[nodiscard]] explicit operator bool() const noexcept { return std::holds_alternative<std::monostate>(error_); }
    [[nodiscard]] const Error& error() const& { return std::get<Error>(error_); }

private:
    Result() = default;
    explicit Result(Error error) : error_(std::move(error)) {}
    std::variant<std::monostate, Error> error_;
};

[[nodiscard]] inline Error makeError(ErrorCode code, std::string message) {
    return Error{code, std::move(message)};
}

} // namespace openlab::core

