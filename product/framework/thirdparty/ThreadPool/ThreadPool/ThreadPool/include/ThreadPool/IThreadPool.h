#ifndef IThreadPool_h__
#define IThreadPool_h__

#include <memory>
#include <string>
#include <functional>
#include <future>
#include <cstdint>
#include <ThreadPool/ThreadPoolExport.h>

namespace ThreadPool {

enum class Priority : uint32_t {
    Urgent = 0,
    High   = 1,
    Normal = 2,
    Low    = 3
};

class THREADPOOL_EXPORT IThreadPool {
public:
    virtual ~IThreadPool() = default;

    virtual void submit(std::function<void()> task, 
                        Priority priority = Priority::Normal,
                        const std::string& tag = "") = 0;

    template<typename Func, typename... Args>
    auto submitWithFuture(Func&& f, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>>
    {
        return submitWithFuturePriority(Priority::Normal, 
                                        std::forward<Func>(f), 
                                        std::forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    auto submitWithFuturePriority(Priority priority, Func&& f, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>>
    {
        using ReturnType = std::invoke_result_t<Func, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(f), std::forward<Args>(args)...)
        );
        auto future = task->get_future();
        submit([task]{ (*task)(); }, priority);
        return future;
    }

    virtual std::string getName() const = 0;
    virtual size_t getPendingTaskCount() const = 0;

    // threadCount = 0 表示自动检测
    static std::shared_ptr<IThreadPool> create(
        uint32_t threadCount = 0,
        const std::string& name = "default"
    );
};

} // namespace ThreadPool

#endif // IThreadPool_h__
