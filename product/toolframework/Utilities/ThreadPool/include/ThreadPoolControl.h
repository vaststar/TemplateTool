#ifndef ThreadPoolControl_h__
#define ThreadPoolControl_h__

#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <map>

namespace ThreadPoolSpace{
class ThreadPool;
class ThreadPoolControl final
{
public:
    static std::shared_ptr<ThreadPoolControl> getInstance();
    void createThreadPool(uint32_t maxpool, const std::string& poolName);
    void createTask(std::function<void()> func, const std::string& functionTag, uint32_t urgentLevel, const std::string& poolName);
private:
    ThreadPoolControl() = default;
    static std::shared_ptr<ThreadPoolControl> _instance;
private:
    std::mutex m_poolMutex;
    std::map<std::string, std::unique_ptr<ThreadPool>> m_threadPools;
public:
    ThreadPoolControl(const ThreadPoolControl &rhs) = delete;
    ThreadPoolControl& operator=(const ThreadPoolControl &rhs) = delete;
    ThreadPoolControl(ThreadPoolControl &&rhs) = delete;
    ThreadPoolControl& operator=(ThreadPoolControl &&rhs) = delete;
    ~ThreadPoolControl();
};
}
#endif // ThreadPoolControl_h__