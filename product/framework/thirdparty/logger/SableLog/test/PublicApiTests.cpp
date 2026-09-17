#include <SableLog/SableLog.h>

#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <filesystem>
#include <memory>
#include <source_location>
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

TEST_CASE("Logger exposes only final-text level methods", "[SableLog][api]")
{
    STATIC_CHECK(FinalTextLogger<sablelog::Logger>);
    STATIC_CHECK_FALSE(AcceptsFormatArguments<sablelog::Logger>);
    STATIC_CHECK_FALSE(std::is_default_constructible_v<sablelog::Logger>);
    STATIC_CHECK(std::is_constructible_v<sablelog::Logger, std::shared_ptr<const sablelog::Runtime>,
                                         std::string, std::string>);
    STATIC_CHECK_FALSE(std::is_constructible_v<sablelog::Logger,
                                               std::shared_ptr<const sablelog::Runtime>, std::string>);
    STATIC_CHECK(std::is_copy_constructible_v<sablelog::Logger>);
    STATIC_CHECK(std::is_move_constructible_v<sablelog::Logger>);
    STATIC_CHECK(RuntimeControl<sablelog::Runtime>);
}

TEST_CASE("console output is configured explicitly", "[SableLog][api]")
{
    sablelog::RuntimeConfig config;
    CHECK(config.loggers.empty());

    sablelog::LoggerConfig loggerConfig;
    loggerConfig.loggerName = "APP";
    CHECK_FALSE(loggerConfig.console.has_value());
    CHECK(loggerConfig.files.empty());

    loggerConfig.console = sablelog::ConsoleConfig{};
    CHECK(loggerConfig.console->minimumLevel == sablelog::Level::Info);
    CHECK(loggerConfig.console->colorMode == sablelog::ConsoleConfig::ColorMode::Automatic);

    sablelog::FileConfig fileConfig;
    using CalendarRotation = sablelog::FileConfig::CalendarRotation;

    STATIC_CHECK(std::same_as<decltype(fileConfig.baseName), std::filesystem::path>);
    STATIC_CHECK(std::same_as<decltype(fileConfig.calendarRotation), CalendarRotation>);
    CHECK(fileConfig.baseName == std::filesystem::path{"application"});
    CHECK(fileConfig.calendarRotation == CalendarRotation::Daily);

    config.loggers.emplace_back(std::move(loggerConfig));
    CHECK(config.loggers.size() == 1U);
}

TEST_CASE("invalid calendar rotation is rejected", "[SableLog][config]")
{
    sablelog::FileConfig fileConfig;
    fileConfig.calendarRotation =
        static_cast<sablelog::FileConfig::CalendarRotation>(0xffU);

    sablelog::LoggerConfig loggerConfig;
    loggerConfig.loggerName = "APP";
    loggerConfig.files.emplace_back(std::move(fileConfig));

    sablelog::RuntimeConfig config;
    config.loggers.emplace_back(std::move(loggerConfig));

    CHECK_THROWS_WITH(sablelog::Runtime{std::move(config)},
                      "SableLog calendar rotation is invalid");
}

} // namespace
