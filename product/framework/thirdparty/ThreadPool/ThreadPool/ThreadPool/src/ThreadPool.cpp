#include "ThreadPool.h"

#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <cstdio>
#include <exception>

namespace ThreadPool {
namespace {
thread_local const void* currentPoolState = nullptr;
} // namespace

//============================================
// IThreadPool::create
//============================================
std::shared_ptr<IThreadPool> IThreadPool::create(uint32_t threadCount, const std::string& name)
{
    uint32_t count = threadCount;
    if (count == 0)
    {
        uint32_t cores = std::thread::hardware_concurrency();
        count = std::min(64u, std::max(4u, cores * 2));
    }
    return std::make_shared<ThreadPool>(count, name);
}

//============================================
// ThreadPoolTask
//============================================
class ThreadPoolTask
{
public:
    ThreadPoolTask() = default;
    ThreadPoolTask(uint32_t taskLevel, const std::string& taskTag, std::function<void()> functionTask)
        : mTaskLevel(taskLevel)
        , mTaskTag(taskTag)
        , mFunction(std::move(functionTask))
    {
    }

    void execute()
    {
        if (mFunction)
        {
            mFunction();
        }
    }

    bool operator>(const uint32_t& rlevel) const
    {
        return mTaskLevel > rlevel;
    }

private:
    uint32_t mTaskLevel{0};
    std::string mTaskTag;
    std::function<void()> mFunction;
};

//============================================
// DataPrivate
//============================================
class ThreadPool::DataPrivate
{
public:
    explicit DataPrivate(const std::string& poolName)
        : mName(poolName)
        , mStop(false)
    {
    }

    ~DataPrivate() = default;

public:
    std::vector<std::thread> mWorkers;
    std::list<ThreadPoolTask> mTasks;
    std::mutex mMutex;
    std::mutex mJoinMutex;
    std::condition_variable mCondition;
    bool mStop;
    std::string mName;
};

//============================================
// ThreadPool
//============================================
ThreadPool::ThreadPool(uint32_t threadCount, const std::string& poolName)
    : mData(std::make_shared<DataPrivate>(poolName))
{
    initPool(std::min<uint32_t>(5000, threadCount));
}

ThreadPool::~ThreadPool()
{
    if (shutdown()) return;

    // A worker cannot join itself. Workers retain State, never the pool object.
    const auto state = mData;
    std::lock_guard<std::mutex> joinLock(state->mJoinMutex);
    for (auto& worker : state->mWorkers)
    {
        if (worker.joinable()) worker.detach();
    }
}

void ThreadPool::initPool(uint32_t poolNumber)
{
    const auto state = mData;
    state->mWorkers.reserve(poolNumber);
    try
    {
        for (uint32_t i = 0; i < poolNumber; ++i)
        {
            state->mWorkers.emplace_back([state]() {
                currentPoolState = state.get();
                while (true)
                {
                    ThreadPoolTask task;
                    {
                        std::unique_lock<std::mutex> lock(state->mMutex);
                        state->mCondition.wait(lock, [&state] {
                            return state->mStop || !state->mTasks.empty();
                        });

                        if (state->mStop && state->mTasks.empty())
                        {
                            currentPoolState = nullptr;
                            return;
                        }

                        task = std::move(state->mTasks.front());
                        state->mTasks.pop_front();
                    }

                    try
                    {
                        task.execute();
                    }
                    catch (const std::exception& e)
                    {
                        std::fprintf(stderr, "ThreadPool [%s]: task failed: %s\n",
                            state->mName.c_str(), e.what());
                    }
                    catch (...)
                    {
                        std::fprintf(stderr, "ThreadPool [%s]: unknown task exception\n",
                            state->mName.c_str());
                    }
                }
            });
        }
    }
    catch (...)
    {
        // Construction may have already started some workers.
        shutdown();
        throw;
    }
}

void ThreadPool::submit(std::function<void()> task, Priority priority, const std::string& tag)
{
    (void)trySubmit(std::move(task), priority, tag);
}

bool ThreadPool::trySubmit(std::function<void()> task, Priority priority, const std::string& tag)
{
    return enqueueFunc(tag, static_cast<uint32_t>(priority), std::move(task));
}

bool ThreadPool::enqueueFunc(const std::string& functionTag, uint32_t urgentLevel, std::function<void()> task)
{
    if (!task) return false;
    const auto state = mData;
    {
        std::unique_lock<std::mutex> lock(state->mMutex);
        if (state->mStop) return false;
        const auto position = std::find_if(state->mTasks.cbegin(), state->mTasks.cend(),
            [urgentLevel](const ThreadPoolTask& item) { return item > urgentLevel; });
        state->mTasks.insert(position,
            ThreadPoolTask(urgentLevel, functionTag, std::move(task)));
    }
    state->mCondition.notify_one();
    return true;
}

std::string ThreadPool::getName() const
{
    return mData->mName;
}

size_t ThreadPool::getPendingTaskCount() const
{
    std::unique_lock<std::mutex> lock(mData->mMutex);
    return mData->mTasks.size();
}

bool ThreadPool::shutdown()
{
    const auto state = mData;
    {
        std::unique_lock<std::mutex> lock(state->mMutex);
        state->mStop = true;
    }
    state->mCondition.notify_all();

    // Must precede mJoinMutex: an external caller may be joining this worker.
    if (currentPoolState == state.get()) return false;

    std::lock_guard<std::mutex> joinLock(state->mJoinMutex);
    for (auto& worker : state->mWorkers)
    {
        if (worker.joinable()) worker.join();
    }
    return true;
}

bool ThreadPool::isShutdown() const
{
    std::unique_lock<std::mutex> lock(mData->mMutex);
    return mData->mStop;
}

} // namespace ThreadPool
