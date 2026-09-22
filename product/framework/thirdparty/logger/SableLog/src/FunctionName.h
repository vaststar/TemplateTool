#pragma once

#include <string>
#include <string_view>

namespace sablelog::detail {

[[nodiscard]] std::string compactFunctionName(std::string_view signature);

} // namespace sablelog::detail
