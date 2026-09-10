#pragma once

#include <cstddef>
#include <mutex>

namespace ucf::utilities::network::detail {

/// Shared byte budget used by all sessions of a server. Queue admission is
/// synchronous so a successful send() has an unambiguous ownership contract.
class QueueBudget final
{
public:
    explicit QueueBudget(std::size_t limit) noexcept
        : mLimit{limit}
    {
    }

    [[nodiscard]] bool tryReserve(std::size_t byteCount) noexcept
    {
        std::scoped_lock lock{mMutex};
        if (byteCount > mLimit - mUsed)
        {
            return false;
        }
        mUsed += byteCount;
        return true;
    }

    void release(std::size_t byteCount) noexcept
    {
        std::scoped_lock lock{mMutex};
        mUsed = byteCount <= mUsed ? mUsed - byteCount : 0;
    }

    [[nodiscard]] std::size_t used() const noexcept
    {
        std::scoped_lock lock{mMutex};
        return mUsed;
    }

private:
    const std::size_t mLimit;
    mutable std::mutex mMutex;
    std::size_t mUsed{0};
};

} // namespace ucf::utilities::network::detail
