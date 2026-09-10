#pragma once

#include <ucf/utilities/NetworkTransportUtils/core/ByteBuffer.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <future>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace network_test {

using namespace std::chrono_literals;
using ucf::utilities::network::ByteBuffer;
using ucf::utilities::network::ByteView;

template<typename T>
class OneShot final
{
public:
    OneShot()
        : mFuture{mPromise.get_future()}
    {
    }

    void set(T value)
    {
        if (!mSet.exchange(true))
        {
            mPromise.set_value(std::move(value));
        }
    }

    [[nodiscard]] bool wait(std::chrono::milliseconds timeout = 3s)
    {
        return mFuture.wait_for(timeout) == std::future_status::ready;
    }

    [[nodiscard]] T get() { return mFuture.get(); }

private:
    std::promise<T> mPromise;
    std::future<T> mFuture;
    std::atomic<bool> mSet{false};
};

template<>
class OneShot<void> final
{
public:
    OneShot()
        : mFuture{mPromise.get_future()}
    {
    }

    void set()
    {
        if (!mSet.exchange(true))
        {
            mPromise.set_value();
        }
    }

    [[nodiscard]] bool wait(std::chrono::milliseconds timeout = 3s)
    {
        return mFuture.wait_for(timeout) == std::future_status::ready;
    }

    void get() { mFuture.get(); }

private:
    std::promise<void> mPromise;
    std::future<void> mFuture;
    std::atomic<bool> mSet{false};
};

class EventCounter final
{
public:
    void signal()
    {
        {
            std::scoped_lock lock{mMutex};
            ++mCount;
        }
        mCondition.notify_all();
    }

    [[nodiscard]] bool waitFor(std::size_t expected, std::chrono::milliseconds timeout = 3s)
    {
        std::unique_lock lock{mMutex};
        return mCondition.wait_for(lock, timeout, [this, expected] {
            return mCount >= expected;
        });
    }

    [[nodiscard]] std::size_t count() const
    {
        std::scoped_lock lock{mMutex};
        return mCount;
    }

private:
    mutable std::mutex mMutex;
    std::condition_variable mCondition;
    std::size_t mCount{0};
};

class ByteCollector final
{
public:
    explicit ByteCollector(std::size_t expected)
        : mExpected{expected}
    {
    }

    void append(ByteView bytes)
    {
        {
            std::scoped_lock lock{mMutex};
            mData.insert(mData.end(), bytes.begin(), bytes.end());
        }
        mCondition.notify_all();
    }

    [[nodiscard]] bool wait(std::chrono::milliseconds timeout = 3s)
    {
        std::unique_lock lock{mMutex};
        return mCondition.wait_for(lock, timeout, [this] {
            return mData.size() >= mExpected;
        });
    }

    [[nodiscard]] ByteBuffer snapshot() const
    {
        std::scoped_lock lock{mMutex};
        return mData;
    }

private:
    const std::size_t mExpected;
    mutable std::mutex mMutex;
    std::condition_variable mCondition;
    ByteBuffer mData;
};

[[nodiscard]] inline ByteBuffer bytes(std::string_view text)
{
    return ByteBuffer{text.begin(), text.end()};
}

} // namespace network_test
