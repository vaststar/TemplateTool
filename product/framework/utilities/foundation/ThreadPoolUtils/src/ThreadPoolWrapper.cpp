#include <ucf/utilities/ThreadPoolUtils/ThreadPoolWrapper.h>
#include <ThreadPool/IThreadPool.h>
#include "ThreadPoolWrapperLogger.h"
#include <thread>
#include <algorithm>
#include <exception>
#include <mutex>
#include <utility>

namespace ucf::utilities {
namespace {

void reportTaskError(const std::string& poolName, const std::string& tag,
                     std::exception_ptr error,
                     const ThreadPoolWrapper::TaskErrorHandler& onError) noexcept
{
    try
    {
        try
        {
            std::rethrow_exception(error);
        }
        catch (const std::exception& e)
        {
            TPWRAPPER_LOG_ERROR("Task exception: pool=" << poolName
                << ", tag=" << tag << ", error=" << e.what());
        }
        catch (...)
        {
            TPWRAPPER_LOG_ERROR("Task exception: pool=" << poolName
                << ", tag=" << tag << ", unknown error");
        }
    }
    catch (...)
    {
        // Logging must not let an exception escape the worker.
    }

    if (onError)
    {
        try
        {
            onError(error);
        }
        catch (...)
        {
            try
            {
                TPWRAPPER_LOG_ERROR("Task error handler failed: pool="
                    << poolName << ", tag=" << tag);
            }
            catch (...)
            {
            }
        }
    }
}

} // namespace

//============================================
// Impl
//============================================
class ThreadPoolWrapper::Impl {
public:
    explicit Impl(uint32_t threadCount, const std::string& name)
        : mName(name)
        , mPool(ThreadPool::IThreadPool::create(threadCount, name))
    {
        const auto actualCount = threadCount == 0
            ? getAutoThreadCount() : std::min(5000u, threadCount);
        TPWRAPPER_LOG_INFO("ThreadPool created: " << mName
            << ", threads=" << actualCount);
    }

    ~Impl()
    {
        // On a worker, the backend preserves State while draining after detach.
        shutdown();
    }

    bool submit(std::function<void()> task, TaskPriority priority,
                const std::string& tag, TaskErrorHandler onError)
    {
        if (!task) return false;

        auto wrappedTask = [task = std::move(task), poolName = mName,
                            tag, onError = std::move(onError)]() noexcept {
            try
            {
                task();
            }
            catch (...)
            {
                reportTaskError(poolName, tag, std::current_exception(), onError);
            }
        };

        std::lock_guard<std::mutex> lock(mMutex);
        if (mStopping || !mPool) return false;
        const auto tpPriority = static_cast<ThreadPool::Priority>(
            static_cast<uint32_t>(priority));
        return mPool->trySubmit(std::move(wrappedTask), tpPriority, tag);
    }

    bool shutdown()
    {
        std::shared_ptr<ThreadPool::IThreadPool> pool;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mStopping = true;
            pool = mPool;
        }
        if (!pool) return true;

        // Never hold the submission mutex while waiting for tasks.
        if (!pool->shutdown()) return false;

        {
            std::lock_guard<std::mutex> lock(mMutex);
            mPool.reset();
        }
        return true;
    }

    std::string getName() const
    {
        return mName;
    }

    size_t getPendingTaskCount() const
    {
        std::shared_ptr<ThreadPool::IThreadPool> pool;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            pool = mPool;
        }
        return pool ? pool->getPendingTaskCount() : 0;
    }

    bool isValid() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return !mStopping && mPool != nullptr;
    }

private:
    static uint32_t getAutoThreadCount()
    {
        const uint32_t cores = std::thread::hardware_concurrency();
        return std::min(64u, std::max(4u, cores * 2));
    }

    const std::string mName;
    mutable std::mutex mMutex;
    std::shared_ptr<ThreadPool::IThreadPool> mPool;
    bool mStopping{false};
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

bool ThreadPoolWrapper::submit(std::function<void()> task,
                               TaskPriority priority,
                               const std::string& tag,
                               TaskErrorHandler onError)
{
    const auto impl = mImpl;
    return impl && impl->submit(std::move(task), priority, tag, std::move(onError));
}

bool ThreadPoolWrapper::shutdown()
{
    const auto impl = mImpl;
    return !impl || impl->shutdown();
}

std::string ThreadPoolWrapper::getName() const
{
    const auto impl = mImpl;
    return impl ? impl->getName() : "";
}

size_t ThreadPoolWrapper::getPendingTaskCount() const
{
    const auto impl = mImpl;
    return impl ? impl->getPendingTaskCount() : 0;
}

bool ThreadPoolWrapper::isValid() const
{
    const auto impl = mImpl;
    return impl && impl->isValid();
}

} // namespace ucf::utilities
