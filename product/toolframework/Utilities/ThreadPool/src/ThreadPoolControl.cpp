#include "ThreadPoolControl.h"

#include <thread>
#include <sstream>
#include <iostream>

#include "ThreadPool.h"
namespace ThreadPoolSpace{
std::shared_ptr<ThreadPoolControl> ThreadPoolControl::_instance = nullptr;
std::shared_ptr<ThreadPoolControl> ThreadPoolControl::getInstance()
{
    static std::once_flag s_flag;
    std::call_once(s_flag, [&]() {
        _instance.reset(new ThreadPoolControl());
    });
    return _instance;
}

ThreadPoolControl::~ThreadPoolControl()
{
    std::scoped_lock<std::mutex> lo(m_poolMutex);
    m_threadPools.clear();
}

void ThreadPoolControl::createThreadPool(uint32_t maxpool, const std::string& poolName)
{
    if (poolName.empty() || 0 == maxpool)
    {
        return;
    }

    std::scoped_lock<std::mutex> lo(m_poolMutex);
    if (!m_threadPools.contains(poolName))
    {
        m_threadPools[poolName] = std::make_unique<ThreadPool>(maxpool, poolName);
    }
}

void ThreadPoolControl::createTask(std::function<void()> func, const std::string& functionTag, uint32_t urgentLevel, const std::string& poolName)
{
    std::scoped_lock<std::mutex> lo(m_poolMutex);
    if (auto iter = m_threadPools.find(poolName); iter != m_threadPools.end())
    {
        iter->second->enqueueFunc(functionTag, urgentLevel, std::move(func));
    }
}
}
