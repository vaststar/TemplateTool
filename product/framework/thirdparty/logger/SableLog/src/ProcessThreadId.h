#pragma once

#include <cstdint>

namespace sablelog::detail {

struct ProcessThreadId final
{
    std::uint64_t processId{};
    std::uint64_t threadId{};
};

[[nodiscard]] ProcessThreadId currentProcessThreadId() noexcept;

} // namespace sablelog::detail
