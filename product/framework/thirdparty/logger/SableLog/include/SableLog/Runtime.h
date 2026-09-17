#pragma once

#include <SableLog/Config.h>
#include <SableLog/SableLogExport.h>

#include <cstdint>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>

namespace sablelog {

class Logger;

// Owns the named logging backends and coordinates their lifetime. An
// integration layer keeps Runtime in a shared_ptr while business code keeps
// Logger handles.
class SABLELOG_API Runtime final
{
public:
    explicit Runtime(RuntimeConfig config);
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    // Waits for all earlier records and asks every output to flush.
    void flush() noexcept;

    // Stops admission, drains accepted records and closes outputs.
    // Repeated calls are safe.
    void shutdown() noexcept;

private:
    class Impl;

    [[nodiscard]] std::uint32_t resolveLogger(std::string_view loggerName) const;
    void submit(std::uint32_t backendId, Level level, std::string_view category,
                std::string_view message, std::source_location location) const noexcept;

    std::unique_ptr<Impl> mImpl;

    friend class Logger;
};

} // namespace sablelog
