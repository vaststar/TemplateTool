#pragma once

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <cstdint>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

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
/// - 轻量包装：只做转发，不增加额外开销
/// 
/// 使用示例：
/// @code
/// ThreadPoolWrapper pool{4, "MyPool"};
/// pool.submit([]{ doWork(); });
/// @endcode
///
class Utilities_EXPORT ThreadPoolWrapper final {
public:
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

    /// 提交任务
    void submit(std::function<void()> task, 
                TaskPriority priority = TaskPriority::Normal,
                const std::string& tag = "");

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
        submit([task]{ (*task)(); }, priority);
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
