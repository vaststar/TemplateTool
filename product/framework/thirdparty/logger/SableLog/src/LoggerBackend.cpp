#include "LoggerBackend.h"

#include "FunctionName.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace sablelog::detail {
namespace {

[[nodiscard]] bool acceptsLevel(Level level, Level minimumLevel) noexcept
{
    if (level == Level::Off || minimumLevel == Level::Off)
    {
        return false;
    }

    return static_cast<std::uint8_t>(level) >= static_cast<std::uint8_t>(minimumLevel);
}

[[nodiscard]] std::string_view levelName(Level level) noexcept
{
    switch (level)
    {
    case Level::Trace:
        return "TRACE";
    case Level::Debug:
        return "DEBUG";
    case Level::Info:
        return "INFO";
    case Level::Warn:
        return "WARN";
    case Level::Error:
        return "ERROR";
    case Level::Fatal:
        return "FATAL";
    case Level::Off:
        return "OFF";
    }

    return "UNKNOWN";
}

[[nodiscard]] std::string_view baseFileName(std::string_view fileName) noexcept
{
    const auto separator = fileName.find_last_of("/\\");
    if (separator == std::string_view::npos)
    {
        return fileName;
    }

    return fileName.substr(separator + 1U);
}

[[nodiscard]] std::string formatUtcTimestamp(
    std::chrono::system_clock::time_point timestamp)
{
    using namespace std::chrono;

    const auto wholeSeconds = floor<seconds>(timestamp);
    const auto fraction = duration_cast<microseconds>(timestamp - wholeSeconds);
    const auto rawTime = system_clock::to_time_t(wholeSeconds);

    std::tm utcTime{};

#if defined(_WIN32)
    if (gmtime_s(&utcTime, &rawTime) != 0)
    {
        throw std::runtime_error{"SableLog UTC timestamp conversion failed"};
    }
#else
    if (gmtime_r(&rawTime, &utcTime) == nullptr)
    {
        throw std::runtime_error{"SableLog UTC timestamp conversion failed"};
    }
#endif

    std::ostringstream stream;
    stream << std::put_time(&utcTime, "%Y-%m-%dT%H:%M:%S")
           << '.' << std::setfill('0') << std::setw(6) << fraction.count() << 'Z';
    return stream.str();
}

} // namespace

LoggerBackend::LoggerBackend(const LoggerConfig& config)
    : mName(config.loggerName), mOutputs(OutputFactory::create(config))
{
    mWorker = std::thread{&LoggerBackend::run, this};
}

LoggerBackend::~LoggerBackend()
{
    shutdown();
}

void LoggerBackend::enqueue(Level level, std::string_view category, std::string_view message,
                            std::source_location location) noexcept
{
    if (!accepts(level))
    {
        return;
    }

    try
    {
        LogRecord record{
            .timestamp = std::chrono::system_clock::now(),
            .threadId = std::this_thread::get_id(),
            .level = level,
            .category = std::string{category},
            .message = std::string{message},
            .fileName = std::string{location.file_name()},
            .functionName = compactFunctionName(location.function_name()),
            .line = location.line(),
            .column = location.column(),
        };

        {
            std::lock_guard lock{mQueueMutex};
            if (mState != State::Running)
            {
                return;
            }

            mQueue.emplace_back(std::move(record));
        }

        mQueueChanged.notify_one();
    }
    catch (...)
    {
        // Logging is best-effort and never propagates failures to the caller.
    }
}

std::future<void> LoggerBackend::requestFlush()
{
    std::promise<void> completion;
    auto future = completion.get_future();

    {
        std::lock_guard lock{mQueueMutex};
        if (mState != State::Running)
        {
            completion.set_value();
            return future;
        }

        mQueue.emplace_back(FlushRequest{std::move(completion)});
    }

    mQueueChanged.notify_one();
    return future;
}

void LoggerBackend::shutdown() noexcept
{
    std::lock_guard shutdownLock{mShutdownMutex};

    {
        std::lock_guard queueLock{mQueueMutex};
        if (mState == State::Running)
        {
            mState = State::Stopping;
        }
    }

    mQueueChanged.notify_all();

    if (mWorker.joinable())
    {
        mWorker.join();
    }

    mOutputs.clear();
}

bool LoggerBackend::accepts(Level level) const noexcept
{
    for (const auto& entry : mOutputs)
    {
        if (entry.output && acceptsLevel(level, entry.minimumLevel))
        {
            return true;
        }
    }

    return false;
}

void LoggerBackend::processRecord(const LogRecord& record) noexcept
{
    std::string renderedLine;

    try
    {
        renderedLine = render(record);
    }
    catch (...)
    {
        return;
    }

    for (auto& entry : mOutputs)
    {
        if (!entry.output || !acceptsLevel(record.level, entry.minimumLevel))
        {
            continue;
        }

        try
        {
            entry.output->write(record.level, renderedLine);
        }
        catch (...)
        {
            // A failed output must not prevent delivery to the remaining outputs.
        }
    }
}

void LoggerBackend::process(QueueItem& item) noexcept
{
    if (auto* record = std::get_if<LogRecord>(&item))
    {
        processRecord(*record);
        return;
    }

    auto& request = std::get<FlushRequest>(item);
    flushOutputs();

    try
    {
        request.completion.set_value();
    }
    catch (...)
    {
        // The waiter may already have abandoned the future.
    }
}

void LoggerBackend::flushOutputs() noexcept
{
    for (auto& entry : mOutputs)
    {
        if (!entry.output)
        {
            continue;
        }

        try
        {
            entry.output->flush();
        }
        catch (...)
        {
            // Continue flushing the remaining outputs.
        }
    }
}

void LoggerBackend::completePendingFlushRequests() noexcept
{
    std::lock_guard lock{mQueueMutex};

    if (mState == State::Running)
    {
        mState = State::Stopping;
    }

    for (auto& item : mQueue)
    {
        auto* request = std::get_if<FlushRequest>(&item);
        if (request == nullptr)
        {
            continue;
        }

        try
        {
            request->completion.set_value();
        }
        catch (...)
        {
            // Completion is best-effort during an unexpected worker failure.
        }
    }

    mQueue.clear();
}

std::string LoggerBackend::render(const LogRecord& record) const
{
    std::ostringstream stream;
    stream << formatUtcTimestamp(record.timestamp)
           << " [" << levelName(record.level) << ']'
           << " [" << record.threadId << ']'
           << " [" << mName << ']'
           << " [" << record.category << ']'
           << " [" << baseFileName(record.fileName) << ':' << record.line;

    if (!record.functionName.empty())
    {
        stream << ' ' << record.functionName;
    }

    stream << "] " << record.message;
    return stream.str();
}

void LoggerBackend::run() noexcept
{
    try
    {
        for (;;)
        {
            std::optional<QueueItem> item;

            {
                std::unique_lock lock{mQueueMutex};
                mQueueChanged.wait(lock, [this] {
                    return mState != State::Running || !mQueue.empty();
                });

                if (mQueue.empty())
                {
                    if (mState != State::Running)
                    {
                        break;
                    }
                    continue;
                }

                item.emplace(std::move(mQueue.front()));
                mQueue.pop_front();
            }

            process(*item);
        }
    }
    catch (...)
    {
        completePendingFlushRequests();
    }

    flushOutputs();

    {
        std::lock_guard lock{mQueueMutex};
        mState = State::Stopped;
    }
}

} // namespace sablelog::detail
