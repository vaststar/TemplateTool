#include "TestSupport.h"

#include <SableLog/SableLog.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace sablelog::test {
namespace {

constexpr std::string_view kLoggerName{"APP"};
constexpr std::string_view kCategory{"TEST"};

class TemporaryDirectory final
{
public:
    TemporaryDirectory()
    {
        const auto root = std::filesystem::temp_directory_path();
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (unsigned attempt = 0U; attempt < 128U; ++attempt)
        {
            mPath = root / ("SableLogTests-" + std::to_string(timestamp) + "-" + std::to_string(attempt));
            if (std::filesystem::create_directory(mPath))
            {
                return;
            }
        }
        throw std::runtime_error{"Could not allocate a unique temporary test directory"};
    }

    ~TemporaryDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(mPath, error);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept
    {
        return mPath;
    }

private:
    std::filesystem::path mPath;
};

void writeText(const std::filesystem::path& path, std::string_view text)
{
    std::ofstream stream{path, std::ios::binary | std::ios::trunc};
    if (!stream.is_open())
    {
        fail("Could not create test file: " + path.string());
    }

    stream.write(text.data(), static_cast<std::streamsize>(text.size()));
    stream.close();

    if (!stream)
    {
        fail("Could not write test file: " + path.string());
    }
}

[[nodiscard]] std::string readText(const std::filesystem::path& path)
{
    std::ifstream stream{path, std::ios::binary};
    if (!stream.is_open())
    {
        fail("Could not open test file: " + path.string());
    }

    std::string text{
        std::istreambuf_iterator<char>{stream},
        std::istreambuf_iterator<char>{},
    };

    if (stream.bad())
    {
        fail("Could not read test file: " + path.string());
    }

    return text;
}

[[nodiscard]] ::sablelog::FileConfig makeFileConfig(
    const std::filesystem::path& directory, std::filesystem::path baseName)
{
    ::sablelog::FileConfig config;
    config.minimumLevel = ::sablelog::Level::Trace;
    config.directory = directory;
    config.baseName = std::move(baseName);
    config.maxFileBytes = 1024U * 1024U;
    config.retentionDays = 0U;
    config.calendarRotation = ::sablelog::FileConfig::CalendarRotation::None;
    return config;
}

[[nodiscard]] ::sablelog::RuntimeConfig makeRuntimeConfig(
    ::sablelog::FileConfig fileConfig, std::string loggerName = std::string{kLoggerName})
{
    ::sablelog::LoggerConfig loggerConfig;
    loggerConfig.loggerName = std::move(loggerName);
    loggerConfig.files.emplace_back(std::move(fileConfig));

    ::sablelog::RuntimeConfig runtimeConfig;
    runtimeConfig.loggers.emplace_back(std::move(loggerConfig));
    return runtimeConfig;
}

[[nodiscard]] bool isExpectedArchiveName(
    const std::filesystem::path& fileName, std::string_view baseName)
{
    const auto text = fileName.filename().generic_string();
    const auto prefix = std::string{baseName} + ".";
    constexpr std::string_view suffix{".0.log"};
    constexpr std::size_t dateLength = 8U;

    if (text.size() != prefix.size() + dateLength + suffix.size() ||
        !text.starts_with(prefix) || !text.ends_with(suffix))
    {
        return false;
    }

    const auto dateBegin = text.begin() + static_cast<std::ptrdiff_t>(prefix.size());
    const auto dateEnd = dateBegin + static_cast<std::ptrdiff_t>(dateLength);
    return std::all_of(dateBegin, dateEnd, [](char value) {
        return value >= '0' && value <= '9';
    });
}

void testOffFileOutputDoesNotCreateFiles()
{
    TemporaryDirectory temporaryDirectory;
    const auto disabledDirectory = temporaryDirectory.path() / "disabled" / "nested";

    auto fileConfig = makeFileConfig(disabledDirectory, "disabled");
    fileConfig.minimumLevel = ::sablelog::Level::Off;

    {
        auto runtime =
            std::make_shared<::sablelog::Runtime>(makeRuntimeConfig(std::move(fileConfig)));
        ::sablelog::Logger logger{runtime, std::string{kLoggerName}, std::string{kCategory}};

        logger.info("ignored before flush");
        runtime->flush();
        runtime->shutdown();

        runtime->flush();
        runtime->shutdown();
        logger.info("ignored after shutdown");
    }

    SABLELOG_EXPECT(!std::filesystem::exists(disabledDirectory));
}

void testArchiveNamespaceConflictIsRejected()
{
    TemporaryDirectory temporaryDirectory;

    auto appFile = makeFileConfig(temporaryDirectory.path(), "application");
    auto conflictingFile =
        makeFileConfig(temporaryDirectory.path(), "application.20260923.0");

    ::sablelog::LoggerConfig appLogger;
    appLogger.loggerName = "APP";
    appLogger.files.emplace_back(std::move(appFile));

    ::sablelog::LoggerConfig mediaLogger;
    mediaLogger.loggerName = "MEDIA";
    mediaLogger.files.emplace_back(std::move(conflictingFile));

    ::sablelog::RuntimeConfig config;
    config.loggers.emplace_back(std::move(appLogger));
    config.loggers.emplace_back(std::move(mediaLogger));

    bool rejected = false;
    try
    {
        static_cast<void>(::sablelog::Runtime{std::move(config)});
    }
    catch (const std::invalid_argument& error)
    {
        rejected = std::string_view{error.what()} ==
                   "SableLog active log file overlaps another output's archive namespace";
    }
    SABLELOG_EXPECT(rejected);

    SABLELOG_EXPECT(!std::filesystem::exists(temporaryDirectory.path() / "application.log"));
    SABLELOG_EXPECT(!std::filesystem::exists(temporaryDirectory.path() / "application.20260923.0.log"));
}

void testFlushWritesAcceptedRecords()
{
    TemporaryDirectory temporaryDirectory;
    const auto activePath = temporaryDirectory.path() / "flush.log";
    constexpr std::string_view marker{"flush-barrier-record-7f42"};

    auto runtime = std::make_shared<::sablelog::Runtime>(
        makeRuntimeConfig(makeFileConfig(temporaryDirectory.path(), "flush")));
    ::sablelog::Logger logger{runtime, std::string{kLoggerName}, std::string{kCategory}};

    logger.info(marker);
    runtime->flush();

    SABLELOG_EXPECT(std::filesystem::exists(activePath));
    SABLELOG_EXPECT(readText(activePath).find(marker) != std::string::npos);

    runtime->shutdown();
}

void testSizeRotationUsesExpectedArchiveName()
{
    TemporaryDirectory temporaryDirectory;
    const auto activePath = temporaryDirectory.path() / "rotation.log";
    constexpr std::string_view firstMarker{"rotation-first-record-22ad"};
    constexpr std::string_view secondMarker{"rotation-second-record-83c1"};

    auto fileConfig = makeFileConfig(temporaryDirectory.path(), "rotation");
    fileConfig.maxFileBytes = 1U;

    auto runtime =
        std::make_shared<::sablelog::Runtime>(makeRuntimeConfig(std::move(fileConfig)));
    ::sablelog::Logger logger{runtime, std::string{kLoggerName}, std::string{kCategory}};

    logger.info(firstMarker);
    logger.info(secondMarker);
    runtime->flush();
    runtime->shutdown();

    SABLELOG_EXPECT(std::filesystem::exists(activePath));

    std::vector<std::filesystem::path> archiveFiles;
    for (const auto& entry : std::filesystem::directory_iterator{temporaryDirectory.path()})
    {
        if (entry.is_regular_file() && entry.path().filename() != activePath.filename())
        {
            archiveFiles.emplace_back(entry.path());
        }
    }
    SABLELOG_EXPECT(archiveFiles.size() == 1U);
    SABLELOG_EXPECT(isExpectedArchiveName(archiveFiles.front().filename(), "rotation"));

    const auto archiveContents = readText(archiveFiles.front());
    const auto activeContents = readText(activePath);

    SABLELOG_EXPECT(archiveContents.find(firstMarker) != std::string::npos);
    SABLELOG_EXPECT(archiveContents.find(secondMarker) == std::string::npos);
    SABLELOG_EXPECT(activeContents.find(firstMarker) == std::string::npos);
    SABLELOG_EXPECT(activeContents.find(secondMarker) != std::string::npos);
}

void testRetentionDeletesOnlyExpiredMatchingArchives()
{
    TemporaryDirectory temporaryDirectory;

    const auto expiredArchive = temporaryDirectory.path() / "retention.20000101.0.log";
    const auto recentArchive = temporaryDirectory.path() / "retention.20000101.1.log";

    const std::vector<std::filesystem::path> unrelatedFiles{
        temporaryDirectory.path() / "retention.20000101.log",
        temporaryDirectory.path() / "retention.20000101.sequence.log",
        temporaryDirectory.path() / "retention.2000101.0.log",
        temporaryDirectory.path() / "retention.200001010.0.log",
        temporaryDirectory.path() / "retention.20000101.0.txt",
        temporaryDirectory.path() / "retention-other.20000101.0.log",
        temporaryDirectory.path() / "retention.20000101.0.log.backup",
    };

    writeText(expiredArchive, "expired archive");
    writeText(recentArchive, "recent archive");

    for (const auto& path : unrelatedFiles)
    {
        writeText(path, "unrelated file");
    }

    using FileDuration = std::filesystem::file_time_type::duration;
    const auto oldAge = std::chrono::duration_cast<FileDuration>(std::chrono::hours{72});
    const auto oldTimestamp = std::filesystem::file_time_type::clock::now() - oldAge;

    std::filesystem::last_write_time(expiredArchive, oldTimestamp);
    for (const auto& path : unrelatedFiles)
    {
        std::filesystem::last_write_time(path, oldTimestamp);
    }

    auto fileConfig = makeFileConfig(temporaryDirectory.path(), "retention");
    fileConfig.retentionDays = 1U;

    {
        ::sablelog::Runtime runtime{makeRuntimeConfig(std::move(fileConfig))};
        runtime.shutdown();
    }

    SABLELOG_EXPECT(!std::filesystem::exists(expiredArchive));
    SABLELOG_EXPECT(std::filesystem::exists(recentArchive));
    SABLELOG_EXPECT(std::filesystem::exists(temporaryDirectory.path() / "retention.log"));

    for (const auto& path : unrelatedFiles)
    {
        SABLELOG_EXPECT(std::filesystem::exists(path));
    }
}

} // namespace

void runFileBehaviorTests()
{
    testOffFileOutputDoesNotCreateFiles();
    testArchiveNamespaceConflictIsRejected();
    testFlushWritesAcceptedRecords();
    testSizeRotationUsesExpectedArchiveName();
    testRetentionDeletesOnlyExpiredMatchingArchives();
}

} // namespace sablelog::test
