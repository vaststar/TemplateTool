#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>

#include <ucf/Infrastructure/TimerScheduler/ITimerScheduler.h>

namespace ucf::infrastructure::scheduling {

class TimerSchedulerImpl final : public ITimerScheduler
{
public:
    TimerSchedulerImpl();
    ~TimerSchedulerImpl() override;

    TimerHandle scheduleOnce(
        std::chrono::milliseconds delay,
        TimerCallback callback) override;

    TimerHandle scheduleAtFixedRate(
        std::chrono::milliseconds initialDelay,
        std::chrono::milliseconds period,
        TimerCallback callback) override;

    TimerHandle scheduleWithFixedDelay(
        std::chrono::milliseconds initialDelay,
        std::chrono::milliseconds period,
        TimerCallback callback) override;

    bool cancel(TimerHandle handle) override;
    void shutdown() override;

private:
    enum class Mode
    {
        OneShot,
        FixedRate,
        FixedDelay
    };

    struct Task
    {
        TimerHandle::IdType id{0};
        std::chrono::steady_clock::time_point nextFire{};
        std::chrono::milliseconds period{0};
        Mode mode{Mode::OneShot};
        TimerCallback callback;
    };

    struct TaskGreater
    {
        bool operator()(
            const std::shared_ptr<Task>& a,
            const std::shared_ptr<Task>& b) const noexcept
        {
            return a->nextFire > b->nextFire;
        }
    };

    TimerHandle enqueue(
        std::chrono::milliseconds initialDelay,
        std::chrono::milliseconds period,
        Mode mode,
        TimerCallback callback);

    void workerLoop();
    void runCallback(const std::shared_ptr<Task>& task) noexcept;

    std::priority_queue<
        std::shared_ptr<Task>,
        std::vector<std::shared_ptr<Task>>,
        TaskGreater> mQueue;
    std::unordered_set<TimerHandle::IdType> mCancelled;
    std::mutex mMutex;
    std::condition_variable mCv;
    std::atomic<TimerHandle::IdType> mNextId{1};
    std::atomic<bool> mRunning{true};
    std::thread mWorker;
};

} // namespace ucf::infrastructure::scheduling
