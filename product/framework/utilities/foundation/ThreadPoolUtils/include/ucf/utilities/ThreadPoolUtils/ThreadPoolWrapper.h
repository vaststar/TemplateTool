#pragma once

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <ucf/utilities/ThreadPoolUtils/ThreadPoolUtilsExport.h>

namespace ucf::utilities {

enum class TaskPriority : uint32_t {
    Urgent = 0,
    High   = 1,
    Normal = 2,
    Low    = 3
};

/// ThreadPoolWrapper - 线程池包装类
/// 
/// 特点：
/// - 拷贝即共享：拷贝 wrapper 后，多个实例共享同一个底层线程池
/// - 普通任务异常隔离，可通过错误回调报告
/// - shutdown 停止所有共享副本的提交，并完成已接收的任务
/// - 不同副本可并发调用；同一对象的赋值/移动/销毁需由调用方同步
/// 
/// 使用示例：
/// @code
/// ThreadPoolWrapper pool{4, "MyPool"};
/// pool.submit([]{ doWork(); });
/// @endcode
///
class THREAD_POOL_UTILS_API ThreadPoolWrapper final {
public:
    using TaskErrorHandler = std::function<void(std::exception_ptr)>;

    /// 创建新的线程池
    /// @param threadCount 线程数，0 = 自动检测
    /// @param name 线程池名称
    explicit ThreadPoolWrapper(uint32_t threadCount = 0, 
                                const std::string& name = "default");

    /// 拷贝构造 - 共享底层线程池
    ThreadPoolWrapper(const ThreadPoolWrapper& other);
    ThreadPoolWrapper& operator=(const ThreadPoolWrapper& other);

    /// 移动构造
    ThreadPoolWrapper(ThreadPoolWrapper&& other) noexcept;
    ThreadPoolWrapper& operator=(ThreadPoolWrapper&& other) noexcept;

    ~ThreadPoolWrapper();

    /// 返回是否接收任务；空任务或已关闭时返回 false，不调用 onError。
    /// onError 在 worker 上执行；它自身的异常也会被隔离。
    bool submit(std::function<void()> task,
                TaskPriority priority = TaskPriority::Normal,
                const std::string& tag = "",
                TaskErrorHandler onError = {});

    /// 停止提交并 drain。外部线程等待所有 workers 退出后返回 true。
    /// 本池 worker 仅请求停止并返回 false；随后可从外部调用等待完成。
    bool shutdown();

    /// 提交带返回值的任务
    template<typename Func, typename... Args>
    auto submitWithFuture(Func&& f, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>>
    {
        return submitWithFuturePriority(TaskPriority::Normal,
                                        std::forward<Func>(f),
                                        std::forward<Args>(args)...);
    }

    template<typename Func, typename... Args>
    auto submitWithFuturePriority(TaskPriority priority, Func&& f, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>>
    {
        using ReturnType = std::invoke_result_t<Func, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(f), std::forward<Args>(args)...)
        );
        auto future = task->get_future();
        if (!submit([task]{ (*task)(); }, priority))
        {
            std::promise<ReturnType> rejected;
            rejected.set_exception(std::make_exception_ptr(
                std::runtime_error("ThreadPool is not accepting tasks")));
            return rejected.get_future();
        }
        return future;
    }

    std::string getName() const;
    size_t getPendingTaskCount() const;
    bool isValid() const;

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
};

} // namespace ucf::utilities
