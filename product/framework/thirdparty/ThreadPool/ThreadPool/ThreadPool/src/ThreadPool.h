#ifndef ThreadPool_h__
#define ThreadPool_h__

#include <memory>
#include <string>
#include <functional>
#include <future>
#include <cstdint>
#include <ThreadPool/IThreadPool.h>

namespace ThreadPool {

class ThreadPool final : public IThreadPool
{
public:
    ThreadPool(uint32_t threadCount, const std::string& poolName);
    ~ThreadPool() override;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    void submit(std::function<void()> task, 
                Priority priority = Priority::Normal,
                const std::string& tag = "") override;
    std::string getName() const override;
    size_t getPendingTaskCount() const override;

    template<typename Fun, typename... Args>
    auto enqueueFutureFunc(const std::string& functionTag, uint32_t urgentLevel, Fun&& f, Args&&... args)
        -> std::future<decltype(std::declval<Fun>()(std::declval<Args>()...))>
    {
        using return_type = decltype(std::declval<Fun>()(std::declval<Args>()...));
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Fun>(f), std::forward<Args>(args)...)
        );
        std::future<return_type> res = task->get_future();
        enqueueFunc(functionTag, urgentLevel, [task]{ (*task)(); });
        return res;
    }

private:
    void initPool(uint32_t poolNumber);
    void enqueueFunc(const std::string& functionTag, uint32_t urgentLevel, std::function<void()> task);
    void shutdown();
    bool isShutdown() const;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mData;
};

} // namespace ThreadPool

#endif // ThreadPool_h__