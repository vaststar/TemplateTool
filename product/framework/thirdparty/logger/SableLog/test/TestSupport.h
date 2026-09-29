#pragma once

#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sablelog::test {

[[noreturn]] inline void fail(std::string_view message, std::source_location location = std::source_location::current())
{
    throw std::runtime_error{std::string{location.file_name()} + ':' + std::to_string(location.line()) +
                             ": " + std::string{message}};
}

inline void expect(bool condition, std::string_view expression, std::source_location location = std::source_location::current())
{
    if (!condition)
    {
        fail(std::string{"check failed: "} + std::string{expression}, location);
    }
}

} // namespace sablelog::test

#define SABLELOG_EXPECT(expression) \
    ::sablelog::test::expect(static_cast<bool>(expression), #expression)
