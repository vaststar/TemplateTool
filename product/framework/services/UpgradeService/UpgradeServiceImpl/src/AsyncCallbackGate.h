#pragma once

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <utility>

namespace ucf::service {

/// Prevents a shutdown from destroying objects while admitted work still uses them.
/// Async callbacks must acquire a Lease before accessing a captured raw pointer.
class AsyncCallbackGate final : public std::enable_shared_from_this<AsyncCallbackGate>
{
public:
    class Lease final
    {
    public:
        Lease() = default;
        Lease(const Lease&) = delete;
        Lease& operator=(const Lease&) = delete;
        Lease(Lease&&) noexcept = default;
        Lease& operator=(Lease&&) = delete;

        ~Lease()
        {
            if (auto gate = std::move(mGate)) {
                gate->leave();
            }
        }

        explicit operator bool() const noexcept { return static_cast<bool>(mGate); }

    private:
        friend class AsyncCallbackGate;
        explicit Lease(std::shared_ptr<AsyncCallbackGate> gate)
            : mGate(std::move(gate)) {}

        std::shared_ptr<AsyncCallbackGate> mGate;
    };

    /// Returns an empty lease after close(). A successful lease keeps this gate
    /// alive and is counted until the callback or operation returns.
    Lease tryEnter()
    {
        auto self = shared_from_this();
        std::lock_guard lock(mMutex);
        if (mClosed) {
            return {};
        }
        ++mActive;
        return Lease(std::move(self));
    }

    void close()
    {
        std::lock_guard lock(mMutex);
        mClosed = true;
    }

    [[nodiscard]] bool isClosed() const
    {
        std::lock_guard lock(mMutex);
        return mClosed;
    }

    /// Call only after close(), and never from a thread holding a Lease.
    void waitForDrain()
    {
        std::unique_lock lock(mMutex);
        mDrainCondition.wait(lock, [this] { return mActive == 0; });
    }

private:
    void leave()
    {
        std::lock_guard lock(mMutex);
        --mActive;
        if (mActive == 0) {
            mDrainCondition.notify_all();
        }
    }

    mutable std::mutex mMutex;
    std::condition_variable mDrainCondition;
    bool mClosed{false};
    std::size_t mActive{0};
};

} // namespace ucf::service
