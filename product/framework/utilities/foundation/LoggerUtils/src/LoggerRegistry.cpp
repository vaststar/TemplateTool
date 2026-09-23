#include "LoggerRegistry.h"

#include "SableLogConfigAdapter.h"

#include <SableLog/Logger.h>
#include <SableLog/Runtime.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace ucf::utilities::detail::logger_registry {
namespace {

struct TransparentStringHash final
{
    using is_transparent = void;

    [[nodiscard]] std::size_t operator()(std::string_view value) const noexcept
    {
        return std::hash<std::string_view>{}(value);
    }

    [[nodiscard]] std::size_t operator()(const std::string& value) const noexcept
    {
        return operator()(std::string_view{value});
    }
};

struct TransparentStringEqual final
{
    using is_transparent = void;

    [[nodiscard]] bool operator()(std::string_view left, std::string_view right) const noexcept
    {
        return left == right;
    }
};

using LoggerHandle = std::shared_ptr<const sablelog::Logger>;
using CategoryLoggers =
    std::unordered_map<std::string, LoggerHandle, TransparentStringHash, TransparentStringEqual>;
using NamedLoggers =
    std::unordered_map<std::string, CategoryLoggers, TransparentStringHash, TransparentStringEqual>;

enum class LifecycleState : std::uint8_t
{
    Uninitialized,
    Running,
    Stopped,
};

struct LoggingSession final
{
    explicit LoggingSession(sablelog::RuntimeConfig config)
    {
        loggersByName.reserve(config.loggers.size());
        for (const auto& loggerConfig : config.loggers)
        {
            loggersByName.try_emplace(loggerConfig.loggerName);
        }

        runtime = std::make_shared<sablelog::Runtime>(std::move(config));
    }

    std::shared_ptr<sablelog::Runtime> runtime;
    std::mutex cacheMutex;
    NamedLoggers loggersByName;
};

struct RegistryState final
{
    std::once_flag exitHandlerRegistration;
    std::mutex lifecycleMutex;
    std::mutex activeMutex;
    std::shared_ptr<LoggingSession> activeSession;
    LifecycleState lifecycle{LifecycleState::Uninitialized};
};

[[nodiscard]] RegistryState& registryState()
{
    // Runtime resources are released by shutdownAtProcessExit. Keeping this small
    // control object alive makes logging from late static destructors a safe no-op.
    static auto* const state = new RegistryState;
    return *state;
}

[[nodiscard]] std::shared_ptr<LoggingSession> activeSession()
{
    auto& state = registryState();
    std::lock_guard activeLock{state.activeMutex};
    return state.activeSession;
}

[[nodiscard]] LoggerHandle findOrCreateCategoryLogger(const std::shared_ptr<LoggingSession>& session,
                                                       std::string_view loggerName,
                                                       std::string_view category)
{
    if (!session || loggerName.empty() || category.empty())
    {
        return {};
    }

    std::lock_guard cacheLock{session->cacheMutex};
    const auto namedLogger = session->loggersByName.find(loggerName);
    if (namedLogger == session->loggersByName.end())
    {
        return {};
    }

    auto& categoryLoggers = namedLogger->second;
    const auto cachedLogger = categoryLoggers.find(category);
    if (cachedLogger != categoryLoggers.end())
    {
        return cachedLogger->second;
    }

    auto logger = std::make_shared<sablelog::Logger>(
        session->runtime, std::string{loggerName}, std::string{category});
    const auto [insertedLogger, inserted] =
        categoryLoggers.emplace(std::string{category}, std::move(logger));
    static_cast<void>(inserted);
    return insertedLogger->second;
}

void dispatch(const sablelog::Logger& logger, LogLevel level, std::string_view message,
              std::source_location location) noexcept
{
    switch (level)
    {
    case LogLevel::Debug:
        logger.debug(message, location);
        return;
    case LogLevel::Info:
        logger.info(message, location);
        return;
    case LogLevel::Warn:
        logger.warn(message, location);
        return;
    case LogLevel::Error:
        logger.error(message, location);
        return;
    case LogLevel::Fatal:
        logger.fatal(message, location);
        return;
    case LogLevel::Off:
        return;
    }
}

void shutdownImpl() noexcept
{
    try
    {
        auto& state = registryState();
        std::lock_guard lifecycleLock{state.lifecycleMutex};
        std::shared_ptr<LoggingSession> stoppingSession;

        if (state.lifecycle == LifecycleState::Stopped)
        {
            return;
        }

        {
            std::lock_guard activeLock{state.activeMutex};
            state.lifecycle = LifecycleState::Stopped;
            stoppingSession = std::move(state.activeSession);
        }

        if (stoppingSession && stoppingSession->runtime)
        {
            stoppingSession->runtime->shutdown();
        }
    }
    catch (...)
    {
        // Shutdown is idempotent and best-effort at the wrapper boundary.
    }
}

void shutdownAtProcessExit() noexcept
{
    shutdownImpl();
}

void registerExitHandler(RegistryState& state)
{
    std::call_once(state.exitHandlerRegistration, [] {
        if (std::atexit(&shutdownAtProcessExit) != 0)
        {
            throw std::runtime_error{"LoggerUtils could not register automatic shutdown"};
        }
    });
}

} // namespace

void initialize(LoggingConfig config)
{
    auto& state = registryState();
    std::lock_guard lifecycleLock{state.lifecycleMutex};

    if (state.lifecycle == LifecycleState::Running)
    {
        throw std::logic_error{"LoggerUtils is already initialized"};
    }
    if (state.lifecycle == LifecycleState::Stopped)
    {
        throw std::logic_error{"LoggerUtils has already been shut down"};
    }

    registerExitHandler(state);
    auto nextSession = std::make_shared<LoggingSession>(toSableLogConfig(std::move(config)));

    {
        std::lock_guard activeLock{state.activeMutex};
        state.lifecycle = LifecycleState::Running;
        state.activeSession = std::move(nextSession);
    }
}

void write(std::string_view loggerName, std::string_view category, LogLevel level,
           std::string_view message, std::source_location location) noexcept
{
    try
    {
        const auto session = activeSession();
        const auto logger = findOrCreateCategoryLogger(session, loggerName, category);
        if (logger)
        {
            dispatch(*logger, level, message, location);
        }
    }
    catch (...)
    {
        // Logging is best-effort and must not affect business execution.
    }
}

void flush() noexcept
{
    try
    {
        const auto session = activeSession();
        if (session && session->runtime)
        {
            session->runtime->flush();
        }
    }
    catch (...)
    {
        // Flushing is best-effort and must not affect business execution.
    }
}

void shutdown() noexcept
{
    shutdownImpl();
}

} // namespace ucf::utilities::detail::logger_registry
