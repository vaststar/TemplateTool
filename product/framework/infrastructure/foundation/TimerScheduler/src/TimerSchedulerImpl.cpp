#include "TimerSchedulerImpl.h"

#include "TimerSchedulerLogger.h"

#include <algorithm>
#include <exception>
#include <string>
#include <utility>

namespace ucf::infrastructure::scheduling {

TimerSchedulerImpl::TimerSchedulerImpl()
{
    mWorker = std::thread{[this]() { workerLoop(); }};
}

TimerSchedulerImpl::~TimerSchedulerImpl()
{
    shutdown();
}

TimerHandle TimerSchedulerImpl::scheduleOnce(
    std::chrono::milliseconds delay,
    TimerCallback callback)
{
    return enqueue(delay, std::chrono::milliseconds{0}, Mode::OneShot, std::move(callback));
}

TimerHandle TimerSchedulerImpl::scheduleAtFixedRate(
    std::chrono::milliseconds initialDelay,
    std::chrono::milliseconds period,
    TimerCallback callback)
{
    return enqueue(initialDelay, period, Mode::FixedRate, std::move(callback));
}

TimerHandle TimerSchedulerImpl::scheduleWithFixedDelay(
    std::chrono::milliseconds initialDelay,
    std::chrono::milliseconds period,
    TimerCallback callback)
{
    return enqueue(initialDelay, period, Mode::FixedDelay, std::move(callback));
}

TimerHandle TimerSchedulerImpl::enqueue(
    std::chrono::milliseconds initialDelay,
    std::chrono::milliseconds period,
    Mode mode,
    TimerCallback callback)
{
    if (!callback)
    {
        TS_LOG_WARN("schedule called with empty callback");
        return TimerHandle{};
    }
    if (mode != Mode::OneShot && period <= std::chrono::milliseconds{0})
    {
        TS_LOG_WARN("periodic schedule requires positive period");
        return TimerHandle{};
    }

    const auto effectiveDelay = std::max(initialDelay, std::chrono::milliseconds{0});

    auto task = std::make_shared<Task>();
    task->id = mNextId.fetch_add(1, std::memory_order_relaxed);
    task->nextFire = std::chrono::steady_clock::now() + effectiveDelay;
    task->period = period;
    task->mode = mode;
    task->callback = std::move(callback);

    {
        std::lock_guard<std::mutex> lk{mMutex};
        if (!mRunning.load(std::memory_order_acquire))
        {
            return TimerHandle{};
        }
        mQueue.push(task);
    }
    mCv.notify_one();
    return TimerHandle{task->id};
}

bool TimerSchedulerImpl::cancel(TimerHandle handle)
{
    if (!handle.isValid())
    {
        return false;
    }
    {
        std::lock_guard<std::mutex> lk{mMutex};
        auto [it, inserted] = mCancelled.insert(handle.id());
        if (!inserted)
        {
            return false;
        }
    }
    mCv.notify_one();
    return true;
}

void TimerSchedulerImpl::shutdown()
{
    mRunning.store(false, std::memory_order_release);
    mCv.notify_all();

    if (mWorker.joinable() && std::this_thread::get_id() != mWorker.get_id())
    {
        mWorker.join();
    }

    std::lock_guard<std::mutex> lk{mMutex};
    while (!mQueue.empty())
    {
        mQueue.pop();
    }
    mCancelled.clear();
}

void TimerSchedulerImpl::workerLoop()
{
    std::unique_lock<std::mutex> lk{mMutex};
    while (mRunning.load(std::memory_order_acquire))
    {
        if (mQueue.empty())
        {
            mCv.wait(lk, [this]() {
                return !mRunning.load(std::memory_order_acquire) || !mQueue.empty();
            });
            continue;
        }

        auto top = mQueue.top();

        if (auto it = mCancelled.find(top->id); it != mCancelled.end())
        {
            mQueue.pop();
            mCancelled.erase(it);
            continue;
        }

        const auto now = std::chrono::steady_clock::now();
        if (top->nextFire > now)
        {
            mCv.wait_until(lk, top->nextFire);
            continue;
        }

        mQueue.pop();
        lk.unlock();
        runCallback(top);
        lk.lock();

        if (!mRunning.load(std::memory_order_acquire))
        {
            continue;
        }
        if (auto it = mCancelled.find(top->id); it != mCancelled.end())
        {
            mCancelled.erase(it);
            continue;
        }
        if (top->mode == Mode::OneShot)
        {
            continue;
        }

        const auto current = std::chrono::steady_clock::now();
        if (top->mode == Mode::FixedRate)
        {
            top->nextFire += top->period;
            // Skip past missed ticks instead of bursting catch-up calls.
            if (top->nextFire <= current)
            {
                const auto behind = current - top->nextFire;
                const auto skips = behind / top->period + 1;
                top->nextFire += top->period * skips;
            }
        }
        else // FixedDelay
        {
            top->nextFire = current + top->period;
        }
        mQueue.push(top);
    }
}

void TimerSchedulerImpl::runCallback(const std::shared_ptr<Task>& task) noexcept
{
    try
    {
        task->callback();
    }
    catch (const std::exception& e)
    {
        TS_LOG_ERROR(std::string{"timer callback threw exception: "} + e.what());
    }
    catch (...)
    {
        TS_LOG_ERROR("timer callback threw unknown exception");
    }
}

} // namespace ucf::infrastructure::scheduling
