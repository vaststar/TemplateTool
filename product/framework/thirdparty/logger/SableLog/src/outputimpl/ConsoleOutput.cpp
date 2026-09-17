#include "ConsoleOutput.h"

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace sablelog::detail {
namespace {

std::mutex consoleOutputMutex;

[[nodiscard]] bool environmentAllowsColor() noexcept
{
    const auto* noColor = std::getenv("NO_COLOR");
    if (noColor != nullptr && noColor[0] != '\0')
    {
        return false;
    }

    const auto* term = std::getenv("TERM");
    return term == nullptr || std::string_view{term} != "dumb";
}

[[nodiscard]] bool terminalSupportsColor() noexcept
{
    if (!environmentAllowsColor())
    {
        return false;
    }

#if defined(_WIN32)
    const auto handle = GetStdHandle(STD_ERROR_HANDLE);
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD mode{};
    if (GetConsoleMode(handle, &mode) == 0)
    {
        return false;
    }

    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0)
    {
        return true;
    }

    return SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
    return isatty(STDERR_FILENO) == 1;
#endif
}

[[nodiscard]] std::string_view colorForLevel(Level level) noexcept
{
    switch (level)
    {
    case Level::Trace:
        return "\x1b[90m";
    case Level::Debug:
        return "\x1b[36m";
    case Level::Info:
        return "\x1b[32m";
    case Level::Warn:
        return "\x1b[33m";
    case Level::Error:
        return "\x1b[31m";
    case Level::Fatal:
        return "\x1b[1;31m";
    case Level::Off:
        return {};
    }

    return {};
}

} // namespace

ConsoleOutput::ConsoleOutput(const ConsoleConfig& config)
{
    switch (config.colorMode)
    {
    case ConsoleConfig::ColorMode::Automatic:
        mUseColor = terminalSupportsColor();
        break;
    case ConsoleConfig::ColorMode::Always:
        mUseColor = true;
        break;
    case ConsoleConfig::ColorMode::Never:
        mUseColor = false;
        break;
    default:
        throw std::invalid_argument{"SableLog console color mode is invalid"};
    }
}

ConsoleOutput::~ConsoleOutput() = default;

void ConsoleOutput::write(Level level, std::string_view renderedLine)
{
    const auto color = mUseColor ? colorForLevel(level) : std::string_view{};
    const auto resetColor = color.empty() ? std::string_view{} : std::string_view{"\x1b[0m"};

    std::string output;
    output.reserve(color.size() + renderedLine.size() + resetColor.size() + 1U);
    output.append(color);
    output.append(renderedLine);
    output.append(resetColor);
    output.push_back('\n');

    const std::lock_guard lock{consoleOutputMutex};
    std::cerr.write(output.data(), static_cast<std::streamsize>(output.size()));

    if (!std::cerr.good())
    {
        throw std::ios_base::failure{"SableLog console output failed"};
    }
}

void ConsoleOutput::flush()
{
    const std::lock_guard lock{consoleOutputMutex};
    std::cerr.flush();

    if (!std::cerr.good())
    {
        throw std::ios_base::failure{"SableLog console flush failed"};
    }
}

} // namespace sablelog::detail
