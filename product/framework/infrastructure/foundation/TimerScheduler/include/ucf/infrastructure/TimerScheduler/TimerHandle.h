#pragma once

#include <cstdint>

namespace ucf::infrastructure::scheduling {

// Opaque identifier for a scheduled task. A default-constructed
// handle is invalid; valid handles have non-zero ids.
class TimerHandle final
{
public:
    using IdType = std::uint64_t;

    constexpr TimerHandle() noexcept = default;
    explicit constexpr TimerHandle(IdType id) noexcept
        : mId{id}
    {
    }

    [[nodiscard]] constexpr IdType id() const noexcept
    {
        return mId;
    }

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return mId != 0;
    }

    friend constexpr bool operator==(const TimerHandle&, const TimerHandle&) noexcept = default;

private:
    IdType mId{0};
};

} // namespace ucf::infrastructure::scheduling
