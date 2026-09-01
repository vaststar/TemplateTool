#include "LogFileLogger.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <string_view>
#include <system_error>

namespace {

std::string getCurrentLocalDate()
{
    const auto now = std::chrono::system_clock::now();
    const auto rawTime =
        std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#if defined(_WIN32)
    if (localtime_s(&localTime, &rawTime) != 0)
    {
        return {};
    }
#else
    if (localtime_r(&rawTime, &localTime) == nullptr)
    {
        return {};
    }
#endif

    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d");
    return stream.str();
}

void reportInternalError(
    std::string_view operation,
    std::string_view detail)
{
    // Do not use MasterLog here because that could recurse back into this logger.
    std::cerr
        << "[MasterLog] LogFileLogger "
        << operation
        << " failed: "
        << detail
        << '\n';
}

std::optional<unsigned int> getRolloverIndex(
    std::string_view fileName,
    std::string_view currentFileName)
{
    if (fileName == currentFileName)
    {
        return 0;
    }

    const std::string rolloverPrefix =
        std::string(currentFileName) + ".";

    if (!fileName.starts_with(rolloverPrefix))
    {
        return std::nullopt;
    }

    const auto indexText =
        fileName.substr(rolloverPrefix.size());

    unsigned int index = 0;
    const auto result = std::from_chars(
        indexText.data(),
        indexText.data() + indexText.size(),
        index);

    if (result.ec != std::errc{} ||
        result.ptr != indexText.data() + indexText.size() ||
        index == 0)
    {
        return std::nullopt;
    }

    return index;
}

} // namespace

namespace LogLogSpace {

LogFileLogger::LogFileLogger(
    int logLevels,
    const std::string& logDirPath,
    const std::string& logBaseName,
    unsigned int maxKeepDays,
    unsigned int maxSingleSize,
    const std::string& loggerName)
    : LogBaseLogger(logLevels, loggerName)
    , m_logDirPath(logDirPath)
    , m_baseFileName(logBaseName)
    , m_maxKeepDays(maxKeepDays)
    , m_maxSingleSize(maxSingleSize)
    , m_currentSize(0)
{
}

LogFileLogger::~LogFileLogger()
{
    stopLog();
    closeCurrentFile();
}

void LogFileLogger::initialize()
{
    std::error_code error;
    std::filesystem::create_directories(
        m_logDirPath,
        error);

    if (error)
    {
        reportInternalError(
            "create log directory",
            error.message());
    }
}

void LogFileLogger::processMessage(
    const std::string& message)
{
    const auto addedSize =
        static_cast<std::uintmax_t>(message.size());

    if (!readyForLog(addedSize) ||
        !m_currentFile.is_open())
    {
        return;
    }

    m_currentFile.write(
        message.data(),
        static_cast<std::streamsize>(message.size()));

    m_currentFile.flush();

    if (!m_currentFile.good())
    {
        reportInternalError(
            "write log file",
            getCurrentLoggerFilePath());

        closeCurrentFile();
        return;
    }

    // Only count bytes that were successfully written and flushed.
    m_currentSize += addedSize;
}

std::vector<std::string>
LogFileLogger::getCurrentFileList() const
{
    std::vector<std::string> result;
    std::error_code error;

    std::filesystem::directory_iterator iterator(
        m_logDirPath,
        error);

    if (error)
    {
        reportInternalError(
            "scan log directory",
            error.message());
        return result;
    }

    const std::filesystem::directory_iterator end;

    while (iterator != end)
    {
        std::error_code entryError;
        const auto& entry = *iterator;

        if (entry.is_regular_file(entryError) &&
            entry.path().filename().string().starts_with(
                m_baseFileName))
        {
            result.emplace_back(entry.path().string());
        }

        if (entryError)
        {
            reportInternalError(
                "inspect log file",
                entryError.message());
        }

        iterator.increment(error);

        if (error)
        {
            reportInternalError(
                "scan log directory",
                error.message());
            break;
        }
    }

    return result;
}

bool LogFileLogger::readyForLog(
    std::uintmax_t addedSize)
{
    const auto currentDate = getCurrentLocalDate();

    if (currentDate.empty())
    {
        reportInternalError(
            "get current date",
            "local time conversion failed");
        return false;
    }

    if (currentDate != m_currentFileDate)
    {
        closeCurrentFile();

        m_currentFileDate = currentDate;

        // Do not carry the previous day's file size into the new day.
        m_currentSize = 0;
    }

    if (!m_currentFile.is_open())
    {
        refreshCurrentFileSize();
    }

    const auto currentFiles = getCurrentFileList();

    removeOldFiles(currentFiles);
    doRollOver(currentFiles, addedSize);

    if (!m_currentFile.is_open())
    {
        return openCurrentFile();
    }

    return true;
}

void LogFileLogger::refreshCurrentFileSize()
{
    m_currentSize = 0;

    const auto filePath = getCurrentLoggerFilePath();
    std::error_code error;

    const bool fileExists =
        std::filesystem::exists(filePath, error);

    if (error)
    {
        reportInternalError(
            "check log file",
            error.message());
        return;
    }

    if (!fileExists)
    {
        return;
    }

    const auto fileSize =
        std::filesystem::file_size(filePath, error);

    if (error)
    {
        reportInternalError(
            "read log file size",
            error.message());
        return;
    }

    m_currentSize = fileSize;
}

bool LogFileLogger::openCurrentFile()
{
    m_currentFile.clear();
    m_currentFile.open(
        getCurrentLoggerFilePath(),
        std::ios::out |
            std::ios::app |
            std::ios::binary);

    if (!m_currentFile.is_open())
    {
        reportInternalError(
            "open log file",
            getCurrentLoggerFilePath());
        return false;
    }

    return true;
}

void LogFileLogger::closeCurrentFile()
{
    if (m_currentFile.is_open())
    {
        m_currentFile.flush();
        m_currentFile.close();
    }

    // Clear failbit in case the previous write failed.
    m_currentFile.clear();
}

void LogFileLogger::removeOldFiles(
    const std::vector<std::string>& allFiles)
{
    std::for_each(
        allFiles.begin(),
        allFiles.end(),
        [this](const std::string& filePath) {
            const std::regex filePattern(
                ".*?" +
                m_baseFileName +
                "-(\\d{4}-\\d{2}-\\d{2})\\.log.*?");

            std::smatch match;
            if (!std::regex_match(
                    filePath,
                    match,
                    filePattern))
            {
                return;
            }

            std::tm fileDate{};
            fileDate.tm_isdst = -1;

            std::istringstream dateStream(
                match[1].str() + " 0:0:0");

            dateStream >> std::get_time(
                &fileDate,
                "%Y-%m-%d %H:%M:%S");

            if (dateStream.fail())
            {
                return;
            }

            const auto fileTime = std::mktime(&fileDate);
            if (fileTime == static_cast<std::time_t>(-1))
            {
                return;
            }

            const auto now =
                std::chrono::system_clock::now();

            const auto currentTime =
                std::chrono::system_clock::to_time_t(now);

            const auto keepSeconds =
                static_cast<double>(m_maxKeepDays) *
                24.0 *
                3600.0;

            // Future-dated files are not expired.
            if (std::difftime(currentTime, fileTime) <=
                keepSeconds)
            {
                return;
            }

            std::error_code error;
            std::filesystem::remove(filePath, error);

            if (error)
            {
                reportInternalError(
                    "remove expired log file",
                    error.message());
            }
        });
}

std::string
LogFileLogger::getCurrentLoggerFilePath() const
{
    return std::filesystem::path(m_logDirPath)
        .append(m_baseFileName)
        .concat("-" + m_currentFileDate + ".log")
        .string();
}

void LogFileLogger::doRollOver(
    const std::vector<std::string>& allFiles,
    std::uintmax_t addedSize)
{
    const auto maxSingleSize =
        static_cast<std::uintmax_t>(m_maxSingleSize);

    const bool shouldRollOver =
        m_currentSize > 0 &&
        (m_currentSize >= maxSingleSize ||
         addedSize > maxSingleSize - m_currentSize);

    if (!shouldRollOver)
    {
        return;
    }

    closeCurrentFile();

    const auto currentPath =
        std::filesystem::path(
            getCurrentLoggerFilePath());

    const auto currentFileName =
        currentPath.filename().string();

    std::vector<
        std::pair<unsigned int, std::filesystem::path>>
        rolloverFiles;

    for (const auto& filePath : allFiles)
    {
        const auto path =
            std::filesystem::path(filePath);

        const auto index = getRolloverIndex(
            path.filename().string(),
            currentFileName);

        if (index.has_value())
        {
            rolloverFiles.emplace_back(*index, path);
        }
    }

    std::sort(
        rolloverFiles.begin(),
        rolloverFiles.end(),
        [](const auto& left, const auto& right) {
            return left.first > right.first;
        });

    for (const auto& [index, sourcePath] :
         rolloverFiles)
    {
        const auto targetPath =
            sourcePath.parent_path() /
            (currentFileName +
             "." +
             std::to_string(index + 1));

        std::error_code error;
        std::filesystem::rename(
            sourcePath,
            targetPath,
            error);

        if (error)
        {
            reportInternalError(
                "roll over log file",
                error.message());

            // Preserve the known size if rollover did not complete.
            return;
        }
    }

    m_currentSize = 0;
}

} // namespace LogLogSpace
