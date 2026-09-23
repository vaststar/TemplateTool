#pragma once

#include "OutputFactory.h"

#include <SableLog/Config.h>

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <future>
#include <mutex>
#include <source_location>
#include <string>
#include <string_view>
#include <thread>
#include <variant>
#include <vector>

namespace sablelog::detail {

class LoggerBackend final
{
public:
    explicit LoggerBackend(const LoggerConfig& config);
    ~LoggerBackend();

    LoggerBackend(const LoggerBackend&) = delete;
    LoggerBackend& operator=(const LoggerBackend&) = delete;
    LoggerBackend(LoggerBackend&&) = delete;
    LoggerBackend& operator=(LoggerBackend&&) = delete;

    void enqueue(Level level, std::string_view category, std::string_view message,
                 std::source_location location) noexcept;

    [[nodiscard]] std::future<void> requestFlush();
    void shutdown() noexcept;

private:
    enum class State : std::uint8_t
    {
        Running,
        Stopping,
        Stopped,
    };

    struct LogRecord final
    {
        std::chrono::system_clock::time_point timestamp;
        std::uint64_t processId;
        std::uint64_t threadId;
        Level level;
        std::string category;
        std::string message;
        std::string fileName;
        std::string functionName;
        std::uint_least32_t line{};
        std::uint_least32_t column{};
    };

    struct FlushRequest final
    {
        std::promise<void> completion;
    };

    using QueueItem = std::variant<LogRecord, FlushRequest>;

    [[nodiscard]] bool accepts(Level level) const noexcept;
    void processRecord(const LogRecord& record) noexcept;
    void process(QueueItem& item) noexcept;
    void flushOutputs() noexcept;
    void completePendingFlushRequests() noexcept;
    [[nodiscard]] std::string render(const LogRecord& record) const;
    void run() noexcept;

    std::string mName;
    std::vector<OutputEntry> mOutputs;

    std::mutex mQueueMutex;
    std::condition_variable mQueueChanged;
    std::deque<QueueItem> mQueue;
    State mState{State::Running};

    std::thread mWorker;
    std::mutex mShutdownMutex;
};

} // namespace sablelog::detail
