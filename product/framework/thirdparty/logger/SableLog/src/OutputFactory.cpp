#include "OutputFactory.h"

#include "FileArchiveName.h"
#include "outputimpl/ConsoleOutput.h"
#include "outputimpl/FileOutput.h"

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

namespace sablelog::detail {
namespace {

[[nodiscard]] bool isKnownLevel(Level level) noexcept
{
    switch (level)
    {
    case Level::Trace:
    case Level::Debug:
    case Level::Info:
    case Level::Warn:
    case Level::Error:
    case Level::Fatal:
    case Level::Off:
        return true;
    }

    return false;
}

void validateMinimumLevel(Level level)
{
    if (!isKnownLevel(level))
    {
        throw std::invalid_argument{"SableLog output minimum level is invalid"};
    }
}

void validateColorMode(ConsoleConfig::ColorMode colorMode)
{
    switch (colorMode)
    {
    case ConsoleConfig::ColorMode::Automatic:
    case ConsoleConfig::ColorMode::Always:
    case ConsoleConfig::ColorMode::Never:
        return;
    }

    throw std::invalid_argument{"SableLog console color mode is invalid"};
}

void validateCalendarRotation(FileConfig::CalendarRotation rotation)
{
    switch (rotation)
    {
    case FileConfig::CalendarRotation::None:
    case FileConfig::CalendarRotation::Daily:
    case FileConfig::CalendarRotation::Monthly:
        return;
    }

    throw std::invalid_argument{"SableLog calendar rotation is invalid"};
}

[[nodiscard]] bool isValidBaseName(const std::filesystem::path& baseName)
{
    return !baseName.empty() && baseName == baseName.filename() &&
           baseName != std::filesystem::path{"."} && baseName != std::filesystem::path{".."};
}

[[nodiscard]] std::filesystem::path normalizeDirectory(const std::filesystem::path& directory)
{
    if (directory.empty())
    {
        throw std::invalid_argument{"SableLog file directory is empty"};
    }

    std::error_code error;
    const auto absoluteDirectory = std::filesystem::absolute(directory, error);
    if (error)
    {
        throw std::filesystem::filesystem_error{
            "SableLog could not resolve the log directory", directory, error};
    }

    auto normalizedDirectory = std::filesystem::weakly_canonical(absoluteDirectory, error);
    if (error)
    {
        throw std::filesystem::filesystem_error{
            "SableLog could not normalize the log directory", absoluteDirectory, error};
    }

    normalizedDirectory.make_preferred();
    return normalizedDirectory;
}

[[nodiscard]] std::filesystem::path activePath(const FileConfig& config)
{
    auto path = config.directory / config.baseName;
    path += ".log";
    path.make_preferred();
    return path.lexically_normal();
}

struct FileTarget final
{
    std::filesystem::path directory;
    std::filesystem::path baseName;
    std::filesystem::path activePath;
};

[[nodiscard]] bool samePath(
    const std::filesystem::path& left, const std::filesystem::path& right)
{
    std::error_code error;
    if (std::filesystem::equivalent(left, right, error))
    {
        return true;
    }

    return left == right;
}

[[nodiscard]] bool overlapsArchiveNamespace(
    const FileTarget& left, const FileTarget& right)
{
    if (!samePath(left.directory, right.directory))
    {
        return false;
    }

    return FileArchiveName::matches(left.activePath.filename(), right.baseName) ||
           FileArchiveName::matches(right.activePath.filename(), left.baseName);
}

} // namespace

void OutputFactory::normalizeAndValidate(RuntimeConfig& config)
{
    std::vector<FileTarget> fileTargets;

    for (auto& logger : config.loggers)
    {
        if (!logger.console.has_value() && logger.files.empty())
        {
            throw std::invalid_argument{"SableLog logger requires at least one output"};
        }

        if (logger.console.has_value())
        {
            validateMinimumLevel(logger.console->minimumLevel);
            validateColorMode(logger.console->colorMode);
        }

        for (auto& file : logger.files)
        {
            validateMinimumLevel(file.minimumLevel);
            validateCalendarRotation(file.calendarRotation);

            if (!isValidBaseName(file.baseName))
            {
                throw std::invalid_argument{"SableLog file base name is invalid"};
            }
            if (file.maxFileBytes == 0U)
            {
                throw std::invalid_argument{"SableLog maximum file size is zero"};
            }

            file.directory = normalizeDirectory(file.directory);

            // Disabled outputs do not create, rotate or remove files and therefore
            // do not participate in active-path conflict detection.
            if (file.minimumLevel == Level::Off)
            {
                continue;
            }

            FileTarget candidate{
                .directory = file.directory,
                .baseName = file.baseName,
                .activePath = activePath(file),
            };

            for (const auto& existing : fileTargets)
            {
                if (samePath(existing.activePath, candidate.activePath))
                {
                    throw std::invalid_argument{
                        "SableLog active log file is configured more than once"};
                }

                if (overlapsArchiveNamespace(existing, candidate))
                {
                    throw std::invalid_argument{
                        "SableLog active log file overlaps another output's archive namespace"};
                }
            }

            fileTargets.emplace_back(std::move(candidate));
        }
    }
}

std::vector<OutputEntry> OutputFactory::create(const LoggerConfig& config)
{
    std::vector<OutputEntry> outputs;
    outputs.reserve(config.files.size() + (config.console.has_value() ? 1U : 0U));

    if (config.console.has_value())
    {
        validateMinimumLevel(config.console->minimumLevel);
        if (config.console->minimumLevel != Level::Off)
        {
            outputs.emplace_back(OutputEntry{
                .minimumLevel = config.console->minimumLevel,
                .output = std::make_unique<ConsoleOutput>(*config.console),
            });
        }
    }

    for (const auto& file : config.files)
    {
        validateMinimumLevel(file.minimumLevel);
        validateCalendarRotation(file.calendarRotation);

        if (file.minimumLevel == Level::Off)
        {
            continue;
        }

        outputs.emplace_back(OutputEntry{
            .minimumLevel = file.minimumLevel,
            .output = std::make_unique<FileOutput>(file),
        });
    }

    return outputs;
}

} // namespace sablelog::detail
