#pragma once

#include <SableLog/SableLogExport.h>

#include <cstdint>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>

namespace sablelog {

class Runtime;

// A cheap, copyable category handle bound to a Runtime. Logger keeps only a
// weak reference, so logging becomes a no-op after that Runtime is destroyed.
// Logger accepts final text only; formatting is deliberately outside SableLog.
class SABLELOG_API Logger final
{
public:
    Logger(std::shared_ptr<const Runtime> runtime, std::string loggerName, std::string category);

    Logger(const Logger& other) noexcept;
    Logger& operator=(const Logger& other) noexcept;
    Logger(Logger&& other) noexcept;
    Logger& operator=(Logger&& other) noexcept;
    ~Logger();

    void trace(std::string_view message, std::source_location location = std::source_location::current()) const noexcept;
    void debug(std::string_view message, std::source_location location = std::source_location::current()) const noexcept;
    void info(std::string_view message, std::source_location location = std::source_location::current()) const noexcept;
    void warn(std::string_view message, std::source_location location = std::source_location::current()) const noexcept;
    void error(std::string_view message, std::source_location location = std::source_location::current()) const noexcept;

    // Fatal is a severity only. It neither terminates the process nor adds an
    // implicit synchronous flush.
    void fatal(std::string_view message, std::source_location location = std::source_location::current()) const noexcept;

private:
    std::weak_ptr<const Runtime> mRuntime;
    std::uint32_t mBackendId{};
    std::string mCategory;
};

} // namespace sablelog
