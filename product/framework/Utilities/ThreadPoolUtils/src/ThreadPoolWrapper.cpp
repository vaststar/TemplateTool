#include <ucf/Utilities/ThreadPoolUtils/ThreadPoolWrapper.h>
#include <ThreadPool/IThreadPool.h>
#include "ThreadPoolWrapperLogger.h"
#include <thread>

namespace ucf::utilities {

//============================================
// Impl
//============================================
class ThreadPoolWrapper::Impl {
public:
    explicit Impl(uint32_t threadCount, const std::string& name)
        : mPool(ThreadPool::IThreadPool::create(threadCount, name))
        , mName(name)
    {
        uint32_t actualCount = (threadCount == 0) ? getAutoThreadCount() : threadCount;
        TPWRAPPER_LOG_INFO("ThreadPool created: " << mName << ", threads=" << actualCount);
    }

    ~Impl()
    {
        TPWRAPPER_LOG_INFO("ThreadPool destroyed: " << mName);
    }

    void submit(std::function<void()> task, TaskPriority priority, const std::string& tag)
    {
        if (!mPool) {
            TPWRAPPER_LOG_ERROR("Submit failed: pool is null, tag=" << tag);
            return;
        }

        TPWRAPPER_LOG_DEBUG("Task submitted: pool=" << mName 
                            << ", tag=" << tag 
                            << ", priority=" << priorityToString(priority)
                            << ", pending=" << mPool->getPendingTaskCount());

        // 包装任务，捕获异常
        auto wrappedTask = [this, task = std::move(task), tag]() {
            try {
                task();
            } catch (const std::exception& e) {
                TPWRAPPER_LOG_ERROR("Task exception: pool=" << mName 
                                    << ", tag=" << tag 
                                    << ", error=" << e.what());
                throw;
            } catch (...) {
                TPWRAPPER_LOG_ERROR("Task exception: pool=" << mName 
                                    << ", tag=" << tag 
                                    << ", unknown error");
                throw;
            }
        };

        auto tpPriority = static_cast<ThreadPool::Priority>(
            static_cast<uint32_t>(priority)
        );
        mPool->submit(std::move(wrappedTask), tpPriority, tag);
    }

    std::string getName() const
    {
        return mPool ? mPool->getName() : "";
    }

    size_t getPendingTaskCount() const
    {
        return mPool ? mPool->getPendingTaskCount() : 0;
    }

    bool isValid() const
    {
        return mPool != nullptr;
    }

private:
    static std::string priorityToString(TaskPriority priority)
    {
        switch (priority) {
            case TaskPriority::Urgent: return "Urgent";
            case TaskPriority::High:   return "High";
            case TaskPriority::Normal: return "Normal";
            case TaskPriority::Low:    return "Low";
            default:                   return "Unknown";
        }
    }

    static uint32_t getAutoThreadCount()
    {
        uint32_t cores = std::thread::hardware_concurrency();
        return std::min(64u, std::max(4u, cores * 2));
    }

private:
    std::shared_ptr<ThreadPool::IThreadPool> mPool;
    std::string mName;
};

//============================================
// ThreadPoolWrapper
//============================================
ThreadPoolWrapper::ThreadPoolWrapper(uint32_t threadCount, const std::string& name)
    : mImpl(std::make_shared<Impl>(threadCount, name))
{
}

ThreadPoolWrapper::ThreadPoolWrapper(const ThreadPoolWrapper& other)
    : mImpl(other.mImpl)
{
}

ThreadPoolWrapper& ThreadPoolWrapper::operator=(const ThreadPoolWrapper& other)
{
    if (this != &other) {
        mImpl = other.mImpl;
    }
    return *this;
}

ThreadPoolWrapper::ThreadPoolWrapper(ThreadPoolWrapper&& other) noexcept
    : mImpl(std::move(other.mImpl))
{
}

ThreadPoolWrapper& ThreadPoolWrapper::operator=(ThreadPoolWrapper&& other) noexcept
{
    if (this != &other) {
        mImpl = std::move(other.mImpl);
    }
    return *this;
}

ThreadPoolWrapper::~ThreadPoolWrapper() = default;

void ThreadPoolWrapper::submit(std::function<void()> task, 
                                TaskPriority priority,
                                const std::string& tag)
{
    if (mImpl) {
        mImpl->submit(std::move(task), priority, tag);
    }
}

std::string ThreadPoolWrapper::getName() const
{
    return mImpl ? mImpl->getName() : "";
}

size_t ThreadPoolWrapper::getPendingTaskCount() const
{
    return mImpl ? mImpl->getPendingTaskCount() : 0;
}

bool ThreadPoolWrapper::isValid() const
{
    return mImpl && mImpl->isValid();
}

} // namespace ucf::utilities
