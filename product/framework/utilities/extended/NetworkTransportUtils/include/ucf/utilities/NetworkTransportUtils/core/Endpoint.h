#pragma once

#include <cstdint>
#include <string>

namespace ucf::utilities::network {

/// Identifies a network endpoint without exposing Asio or native socket types.
///
/// host may be a DNS name or a numeric IPv4/IPv6 address. A bind operation may
/// interpret an empty host as all local interfaces. Port zero remains valid for
/// bind operations that ask the operating system to select an available port.
struct Endpoint final
{
    std::string host;
    std::uint16_t port{0};

    [[nodiscard]] friend bool operator==(const Endpoint&, const Endpoint&) = default;
};

} // namespace ucf::utilities::network
