#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace ucf::service::chat {

inline constexpr std::size_t maxTextBytes = 4096;

std::int64_t nowMs();
std::string randomId();
bool validText(std::string_view text, std::size_t maximum);

} // namespace ucf::service::chat
