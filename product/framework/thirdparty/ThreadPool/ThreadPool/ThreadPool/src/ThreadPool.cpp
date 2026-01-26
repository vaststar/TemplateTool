#include "ThreadPool.h"

#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

namespace ThreadPool {

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
    std::condition_variable mCondition;
    bool mStop;
    std::string mName;
};

//============================================
// ThreadPool
//============================================
ThreadPool::ThreadPool(uint32_t threadCount, const std::string& poolName)
    : mData(std::make_unique<DataPrivate>(poolName))
{
    initPool(std::min<uint32_t>(5000, threadCount));
}

ThreadPool::~ThreadPool()
{
    shutdown();
}

void ThreadPool::initPool(uint32_t poolNumber)
{
    for (uint32_t i = 0; i < poolNumber; ++i)
    {
        mData->mWorkers.emplace_back([this]() {
            while (true)
            {
                ThreadPoolTask task;
                {
                    std::unique_lock<std::mutex> lock(mData->mMutex);
                    mData->mCondition.wait(lock, [this] {
                        return mData->mStop || !mData->mTasks.empty();
                    });

                    if (mData->mStop && mData->mTasks.empty())
                    {
                        return;
                    }

                    task = std::move(mData->mTasks.front());
                    mData->mTasks.pop_front();
                }
                task.execute();
            }
        });
    }
}

void ThreadPool::submit(std::function<void()> task, Priority priority, const std::string& tag)
{
    enqueueFunc(tag, static_cast<uint32_t>(priority), std::move(task));
}

void ThreadPool::enqueueFunc(const std::string& functionTag, uint32_t urgentLevel, std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(mData->mMutex);
        if (mData->mStop)
        {
            return;
        }
        auto enqueItor = std::find_if(mData->mTasks.cbegin(), mData->mTasks.cend(),
            [urgentLevel](const ThreadPoolTask& taskItem) {
                return taskItem > urgentLevel;
            });
        mData->mTasks.insert(enqueItor, ThreadPoolTask(urgentLevel, functionTag, std::move(task)));
    }
    mData->mCondition.notify_one();
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

void ThreadPool::shutdown()
{
    {
        std::unique_lock<std::mutex> lock(mData->mMutex);
        if (mData->mStop)
        {
            return;
        }
        mData->mStop = true;
    }
    mData->mCondition.notify_all();

    for (auto& worker : mData->mWorkers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

bool ThreadPool::isShutdown() const
{
    std::unique_lock<std::mutex> lock(mData->mMutex);
    return mData->mStop;
}

} // namespace ThreadPool
