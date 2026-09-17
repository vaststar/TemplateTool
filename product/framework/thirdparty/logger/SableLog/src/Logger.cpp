#include <SableLog/Logger.h>
#include <SableLog/Runtime.h>

#include <stdexcept>
#include <utility>

namespace sablelog {

Logger::Logger(std::shared_ptr<const Runtime> runtime, std::string loggerName, std::string category)
{
    if (!runtime)
    {
        throw std::invalid_argument{"SableLog logger runtime is null"};
    }
    if (category.empty())
    {
        throw std::invalid_argument{"SableLog logger category is empty"};
    }

    mBackendId = runtime->resolveLogger(loggerName);
    mRuntime = runtime;
    mCategory = std::move(category);
}

Logger::Logger(const Logger& other) noexcept = default;
Logger& Logger::operator=(const Logger& other) noexcept = default;
Logger::Logger(Logger&& other) noexcept = default;
Logger& Logger::operator=(Logger&& other) noexcept = default;
Logger::~Logger() = default;

void Logger::trace(std::string_view message, std::source_location location) const noexcept
{
    if (const auto runtime = mRuntime.lock())
    {
        runtime->submit(mBackendId, Level::Trace, mCategory, message, location);
    }
}

void Logger::debug(std::string_view message, std::source_location location) const noexcept
{
    if (const auto runtime = mRuntime.lock())
    {
        runtime->submit(mBackendId, Level::Debug, mCategory, message, location);
    }
}

void Logger::info(std::string_view message, std::source_location location) const noexcept
{
    if (const auto runtime = mRuntime.lock())
    {
        runtime->submit(mBackendId, Level::Info, mCategory, message, location);
    }
}

void Logger::warn(std::string_view message, std::source_location location) const noexcept
{
    if (const auto runtime = mRuntime.lock())
    {
        runtime->submit(mBackendId, Level::Warn, mCategory, message, location);
    }
}

void Logger::error(std::string_view message, std::source_location location) const noexcept
{
    if (const auto runtime = mRuntime.lock())
    {
        runtime->submit(mBackendId, Level::Error, mCategory, message, location);
    }
}

void Logger::fatal(std::string_view message, std::source_location location) const noexcept
{
    if (const auto runtime = mRuntime.lock())
    {
        runtime->submit(mBackendId, Level::Fatal, mCategory, message, location);
    }
}

} // namespace sablelog
