#include "TestSupport.h"

#include <SableLog/SableLog.h>

#include <concepts>
#include <filesystem>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace sablelog::test {
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

void testLoggerFinalTextApi()
{
    static_assert(FinalTextLogger<::sablelog::Logger>);
    static_assert(!AcceptsFormatArguments<::sablelog::Logger>);
    static_assert(!std::is_default_constructible_v<::sablelog::Logger>);
    static_assert(std::is_constructible_v<::sablelog::Logger, std::shared_ptr<const ::sablelog::Runtime>,
                                          std::string, std::string>);
    static_assert(!std::is_constructible_v<::sablelog::Logger,
                                           std::shared_ptr<const ::sablelog::Runtime>, std::string>);
    static_assert(std::is_copy_constructible_v<::sablelog::Logger>);
    static_assert(std::is_move_constructible_v<::sablelog::Logger>);
    static_assert(RuntimeControl<::sablelog::Runtime>);
}

void testConsoleOutputConfiguration()
{
    ::sablelog::RuntimeConfig config;
    SABLELOG_EXPECT(config.loggers.empty());

    ::sablelog::LoggerConfig loggerConfig;
    loggerConfig.loggerName = "APP";
    SABLELOG_EXPECT(!loggerConfig.console.has_value());
    SABLELOG_EXPECT(loggerConfig.files.empty());

    loggerConfig.console = ::sablelog::ConsoleConfig{};
    SABLELOG_EXPECT(loggerConfig.console->minimumLevel == ::sablelog::Level::Info);
    SABLELOG_EXPECT(
        loggerConfig.console->colorMode == ::sablelog::ConsoleConfig::ColorMode::Automatic);

    ::sablelog::FileConfig fileConfig;
    using CalendarRotation = ::sablelog::FileConfig::CalendarRotation;

    static_assert(std::same_as<decltype(fileConfig.baseName), std::filesystem::path>);
    static_assert(std::same_as<decltype(fileConfig.calendarRotation), CalendarRotation>);
    SABLELOG_EXPECT(fileConfig.baseName == std::filesystem::path{"application"});
    SABLELOG_EXPECT(fileConfig.calendarRotation == CalendarRotation::Daily);

    config.loggers.emplace_back(std::move(loggerConfig));
    SABLELOG_EXPECT(config.loggers.size() == 1U);
}

void testInvalidCalendarRotation()
{
    ::sablelog::FileConfig fileConfig;
    fileConfig.calendarRotation = static_cast<::sablelog::FileConfig::CalendarRotation>(0xffU);

    ::sablelog::LoggerConfig loggerConfig;
    loggerConfig.loggerName = "APP";
    loggerConfig.files.emplace_back(std::move(fileConfig));

    ::sablelog::RuntimeConfig config;
    config.loggers.emplace_back(std::move(loggerConfig));

    bool rejected = false;
    try
    {
        static_cast<void>(::sablelog::Runtime{std::move(config)});
    }
    catch (const std::invalid_argument& error)
    {
        rejected = std::string_view{error.what()} == "SableLog calendar rotation is invalid";
    }
    SABLELOG_EXPECT(rejected);
}

} // namespace

void runPublicApiTests()
{
    testLoggerFinalTextApi();
    testConsoleOutputConfiguration();
    testInvalidCalendarRotation();
}

} // namespace sablelog::test
