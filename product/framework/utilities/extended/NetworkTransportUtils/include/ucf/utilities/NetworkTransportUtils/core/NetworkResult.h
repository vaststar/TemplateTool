#pragma once

#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>

#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace ucf::utilities::network {

/// Holds either a successful value or a structured network error.
template<typename T>
class [[nodiscard]] NetworkResult final
{
    static_assert(!std::is_reference_v<T>, "NetworkResult does not support reference values");

public:
    [[nodiscard]] static NetworkResult success(T value) { return NetworkResult{SuccessTag{}, std::move(value)}; }
    [[nodiscard]] static NetworkResult failure(NetworkError error) { return NetworkResult{std::move(error)}; }

    [[nodiscard]] bool hasValue() const noexcept { return mStorage.index() == 0; }
    [[nodiscard]] explicit operator bool() const noexcept { return hasValue(); }

    [[nodiscard]] T& value() & { return std::get<0>(mStorage); }
    [[nodiscard]] const T& value() const& { return std::get<0>(mStorage); }
    [[nodiscard]] T&& value() && { return std::get<0>(std::move(mStorage)); }

    [[nodiscard]] NetworkError& error() & { return std::get<1>(mStorage); }
    [[nodiscard]] const NetworkError& error() const& { return std::get<1>(mStorage); }
    [[nodiscard]] NetworkError&& error() && { return std::get<1>(std::move(mStorage)); }

private:
    struct SuccessTag final
    {
    };

    explicit NetworkResult(SuccessTag, T value)
        : mStorage{std::in_place_index<0>, std::move(value)}
    {
    }

    explicit NetworkResult(NetworkError error)
        : mStorage{std::in_place_index<1>, std::move(error)}
    {
    }

private:
    std::variant<T, NetworkError> mStorage;
};

/// Success-or-error specialization for commands that have no return value.
template<>
class [[nodiscard]] NetworkResult<void> final
{
public:
    [[nodiscard]] static NetworkResult success() { return NetworkResult{}; }
    [[nodiscard]] static NetworkResult failure(NetworkError error) { return NetworkResult{std::move(error)}; }

    [[nodiscard]] bool hasValue() const noexcept { return !mError.has_value(); }
    [[nodiscard]] explicit operator bool() const noexcept { return hasValue(); }

    [[nodiscard]] NetworkError& error() & { return mError.value(); }
    [[nodiscard]] const NetworkError& error() const& { return mError.value(); }
    [[nodiscard]] NetworkError&& error() && { return std::move(mError).value(); }

private:
    NetworkResult() = default;

    explicit NetworkResult(NetworkError error)
        : mError{std::move(error)}
    {
    }

private:
    std::optional<NetworkError> mError;
};

} // namespace ucf::utilities::network
