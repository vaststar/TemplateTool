#include <SableLog/SableLog.h>

#include <array>
#include <concepts>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

template<typename T>
concept FinalTextLogger = requires(const T& logger, std::string_view text, std::source_location location)
{
    { logger.trace(text, location) } noexcept -> std::same_as<void>;
    { logger.debug(text, location) } noexcept -> std::same_as<void>;
    { logger.info(text, location) } noexcept -> std::same_as<void>;
    { logger.warn(text, location) } noexcept -> std::same_as<void>;
    { logger.error(text, location) } noexcept -> std::same_as<void>;
    { logger.fatal(text, location) } noexcept -> std::same_as<void>;
};

template<typename T>
concept AcceptsFormatArguments = requires(const T& logger)
{
    logger.info("value={}", 42);
};

template<typename T>
concept RuntimeControl = requires(T& runtime)
{
    { runtime.flush() } noexcept -> std::same_as<void>;
    { runtime.shutdown() } noexcept -> std::same_as<void>;
};

class TestFailure final : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

[[noreturn]] void fail(std::string_view message,
                       const std::source_location location = std::source_location::current())
{
    std::string diagnostic{location.file_name()};
    diagnostic += ':';
    diagnostic += std::to_string(location.line());
    diagnostic += ": ";
    diagnostic += message;
    throw TestFailure{std::move(diagnostic)};
}

void expect(const bool condition, std::string_view expression,
            const std::source_location location = std::source_location::current())
{
    if (!condition)
    {
        std::string message{"check failed: "};
        message += expression;
        fail(message, location);
    }
}

template<typename Function>
void expectThrowsWith(Function&& function, std::string_view expectedMessage,
                      const std::source_location location = std::source_location::current())
{
    try
    {
        std::forward<Function>(function)();
    }
    catch (const std::exception& error)
    {
        if (expectedMessage == error.what())
        {
            return;
        }

        std::string message{"expected exception message \""};
        message += expectedMessage;
        message += "\", got \"";
        message += error.what();
        message += '"';
        fail(message, location);
    }
    catch (...)
    {
        fail("expected a standard exception", location);
    }

    fail("expected an exception, but none was thrown", location);
}

#define SABLELOG_EXPECT(expression) expect(static_cast<bool>(expression), #expression)

void testLoggerFinalTextApi()
{
    static_assert(FinalTextLogger<sablelog::Logger>);
    static_assert(!AcceptsFormatArguments<sablelog::Logger>);
    static_assert(!std::is_default_constructible_v<sablelog::Logger>);
    static_assert(std::is_constructible_v<sablelog::Logger,
                                          std::shared_ptr<const sablelog::Runtime>, std::string,
                                          std::string>);
    static_assert(!std::is_constructible_v<sablelog::Logger,
                                           std::shared_ptr<const sablelog::Runtime>, std::string>);
    static_assert(std::is_copy_constructible_v<sablelog::Logger>);
    static_assert(std::is_move_constructible_v<sablelog::Logger>);
    static_assert(RuntimeControl<sablelog::Runtime>);
}

void testConsoleOutputConfiguration()
{
    sablelog::RuntimeConfig config;
    SABLELOG_EXPECT(config.loggers.empty());

    sablelog::LoggerConfig loggerConfig;
    loggerConfig.loggerName = "APP";
    SABLELOG_EXPECT(!loggerConfig.console.has_value());
    SABLELOG_EXPECT(loggerConfig.files.empty());

    loggerConfig.console = sablelog::ConsoleConfig{};
    SABLELOG_EXPECT(loggerConfig.console->minimumLevel == sablelog::Level::Info);
    SABLELOG_EXPECT(loggerConfig.console->colorMode
                    == sablelog::ConsoleConfig::ColorMode::Automatic);

    sablelog::FileConfig fileConfig;
    using CalendarRotation = sablelog::FileConfig::CalendarRotation;

    static_assert(std::same_as<decltype(fileConfig.baseName), std::filesystem::path>);
    static_assert(std::same_as<decltype(fileConfig.calendarRotation), CalendarRotation>);
    SABLELOG_EXPECT(fileConfig.baseName == std::filesystem::path{"application"});
    SABLELOG_EXPECT(fileConfig.calendarRotation == CalendarRotation::Daily);

    config.loggers.emplace_back(std::move(loggerConfig));
    SABLELOG_EXPECT(config.loggers.size() == 1U);
}

void testInvalidCalendarRotation()
{
    sablelog::FileConfig fileConfig;
    fileConfig.calendarRotation =
        static_cast<sablelog::FileConfig::CalendarRotation>(0xffU);

    sablelog::LoggerConfig loggerConfig;
    loggerConfig.loggerName = "APP";
    loggerConfig.files.emplace_back(std::move(fileConfig));

    sablelog::RuntimeConfig config;
    config.loggers.emplace_back(std::move(loggerConfig));

    expectThrowsWith(
        [&config] { static_cast<void>(sablelog::Runtime{std::move(config)}); },
        "SableLog calendar rotation is invalid");
}

#undef SABLELOG_EXPECT

struct TestCase
{
    std::string_view id;
    std::string_view name;
    void (*run)();
};

constexpr std::array<TestCase, 3> testCases{{
    {"logger-final-text-api", "Logger exposes only final-text level methods",
     &testLoggerFinalTextApi},
    {"console-output-configuration", "console output is configured explicitly",
     &testConsoleOutputConfiguration},
    {"invalid-calendar-rotation", "invalid calendar rotation is rejected",
     &testInvalidCalendarRotation},
}};

int runTest(const TestCase& test) noexcept
{
    try
    {
        test.run();
        std::cout << "[PASS] " << test.name << '\n';
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "[FAIL] " << test.name << '\n' << error.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "[FAIL] " << test.name << "\nunexpected non-standard exception\n";
    }

    return 1;
}

} // namespace

int main(const int argc, char* argv[])
{
    if (argc == 1)
    {
        int failures = 0;
        for (const auto& test : testCases)
        {
            failures += runTest(test);
        }
        return failures == 0 ? 0 : 1;
    }

    if (argc == 2)
    {
        const std::string_view requestedTest{argv[1]};
        for (const auto& test : testCases)
        {
            if (test.id == requestedTest)
            {
                return runTest(test);
            }
        }
    }

    std::cerr << "usage: " << argv[0]
              << " [logger-final-text-api|console-output-configuration|invalid-calendar-rotation]\n";
    return 2;
}
