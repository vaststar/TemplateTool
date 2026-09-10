#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace ucf::utilities::network {

/// Owning binary data used by network transports.
using ByteBuffer = std::vector<std::uint8_t>;

/// Non-owning binary data valid only for the duration documented by an API.
using ByteView = std::span<const std::uint8_t>;

} // namespace ucf::utilities::network
