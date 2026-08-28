#pragma once

#include <string>
#include <utility>
#include <variant>

namespace ucf::utilities {

enum class TimeErrorCode
{
    InvalidDate,
    InvalidTime,
    InvalidDateTime,
    InvalidFormat,
    OutOfRange,
    UnknownTimeZone,
    TimeZoneNotSupported,
    TimeZoneDatabaseUnavailable,
    AmbiguousLocalTime,
    NonexistentLocalTime
};

struct TimeError final
{
    TimeErrorCode code;
    // Stable program decisions should use code. diagnostic is non-localized
    // troubleshooting context intended for logs and developer tooling.
    std::string diagnostic;
};

template<typename T>
class [[nodiscard]] TimeResult final
{
public:
    [[nodiscard]] static TimeResult success(T value)
    {
        return TimeResult{std::move(value)};
    }

    [[nodiscard]] static TimeResult failure(TimeErrorCode code, std::string diagnostic)
    {
        return TimeResult{TimeError{code, std::move(diagnostic)}};
    }

    [[nodiscard]] bool hasValue() const noexcept
    {
        return std::holds_alternative<T>(mStorage);
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return hasValue();
    }

    [[nodiscard]] const T& value() const&
    {
        return std::get<T>(mStorage);
    }

    [[nodiscard]] T&& value() &&
    {
        return std::get<T>(std::move(mStorage));
    }

    [[nodiscard]] const TimeError& error() const&
    {
        return std::get<TimeError>(mStorage);
    }

private:
    explicit TimeResult(T value)
        : mStorage{std::move(value)}
    {
    }

    explicit TimeResult(TimeError error)
        : mStorage{std::move(error)}
    {
    }

private:
    std::variant<T, TimeError> mStorage;
};

} // namespace ucf::utilities
