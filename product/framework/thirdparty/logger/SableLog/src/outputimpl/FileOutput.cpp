#include "FileOutput.h"

#include "../FileArchiveName.h"

#include <chrono>
#include <iomanip>
#include <ios>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace sablelog::detail {
namespace {

[[nodiscard]] std::chrono::sys_days currentUtcDay() noexcept
{
    return std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
}

[[nodiscard]] std::string formatUtcDate(std::chrono::sys_days day)
{
    const std::chrono::year_month_day date{day};
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text << std::setfill('0') << std::setw(4) << static_cast<int>(date.year()) << std::setw(2)
         << static_cast<unsigned>(date.month()) << std::setw(2)
         << static_cast<unsigned>(date.day());

    if (!text.good())
    {
        throw std::runtime_error{"SableLog could not format the UTC archive date"};
    }

    return text.str();
}

template <typename Clock, typename Duration>
[[nodiscard]] auto toSystemTime(std::chrono::time_point<Clock, Duration> timestamp)
{
    if constexpr (requires { Clock::to_sys(timestamp); })
    {
        return Clock::to_sys(timestamp);
    }
    else
    {
        using UtcTimePoint = decltype(Clock::to_utc(timestamp));
        using UtcClock = typename UtcTimePoint::clock;

        const auto utcTimestamp = Clock::to_utc(timestamp);
        return UtcClock::to_sys(utcTimestamp);
    }
}

[[nodiscard]] std::chrono::sys_days toUtcDay(std::filesystem::file_time_type timestamp)
{
    return std::chrono::floor<std::chrono::days>(toSystemTime(timestamp));
}

[[nodiscard]] bool crossedCalendarBoundary(FileConfig::CalendarRotation rotation,
                                           std::chrono::sys_days lastWriteDay,
                                           std::chrono::sys_days currentDay)
{
    switch (rotation)
    {
    case FileConfig::CalendarRotation::None:
        return false;

    case FileConfig::CalendarRotation::Daily:
        return lastWriteDay != currentDay;

    case FileConfig::CalendarRotation::Monthly:
    {
        const std::chrono::year_month_day lastWriteDate{lastWriteDay};
        const std::chrono::year_month_day currentDate{currentDay};

        return lastWriteDate.year() != currentDate.year() ||
               lastWriteDate.month() != currentDate.month();
    }
    }

    throw std::logic_error{"SableLog calendar rotation invariant is invalid"};
}

[[nodiscard]] bool isValidBaseName(const std::filesystem::path& baseName)
{
    return !baseName.empty() && baseName == baseName.filename() &&
           baseName != std::filesystem::path{"."} && baseName != std::filesystem::path{".."};
}

} // namespace

FileOutput::FileOutput(const FileConfig& config) : mConfig(config)
{
    if (mConfig.directory.empty())
    {
        throw std::invalid_argument{"SableLog file directory is empty"};
    }
    if (!isValidBaseName(mConfig.baseName))
    {
        throw std::invalid_argument{"SableLog file base name is invalid"};
    }
    if (mConfig.maxFileBytes == 0U)
    {
        throw std::invalid_argument{"SableLog maximum file size is zero"};
    }

    std::error_code error;
    std::filesystem::create_directories(mConfig.directory, error);
    if (error)
    {
        throw std::filesystem::filesystem_error{
            "SableLog could not create the log directory", mConfig.directory, error};
    }

    if (!std::filesystem::is_directory(mConfig.directory, error) || error)
    {
        if (!error)
        {
            error = std::make_error_code(std::errc::not_a_directory);
        }
        throw std::filesystem::filesystem_error{
            "SableLog log directory is not a directory", mConfig.directory, error};
    }

    mActivePath = mConfig.directory / mConfig.baseName;
    mActivePath += ".log";
    removeExpiredArchives();
    openActiveFile();

    const auto currentDay = currentUtcDay();
    if (mCurrentSize != 0U)
    {
        const auto shouldRotateForCalendar =
            crossedCalendarBoundary(mConfig.calendarRotation, mLastWriteDay, currentDay);
        const auto shouldRotateForSize = mCurrentSize >= mConfig.maxFileBytes;

        if (shouldRotateForCalendar || shouldRotateForSize)
        {
            rotate();
        }
    }
}

FileOutput::~FileOutput() = default;

void FileOutput::write(Level, std::string_view renderedLine)
{
    if (renderedLine.size() >= std::numeric_limits<std::uint64_t>::max())
    {
        throw std::length_error{"SableLog file record is too large"};
    }
    if (renderedLine.size() > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()))
    {
        throw std::length_error{"SableLog file record exceeds the stream size limit"};
    }

    const auto recordBytes = static_cast<std::uint64_t>(renderedLine.size()) + 1U;

    try
    {
        if (!mStream.is_open())
        {
            openActiveFile();
        }

        const auto currentDay = currentUtcDay();
        rotateIfNeeded(recordBytes, currentDay);

        mStream.write(renderedLine.data(), static_cast<std::streamsize>(renderedLine.size()));
        mStream.put('\n');
        mStream.flush();

        if (!mStream.good())
        {
            throw std::ios_base::failure{"SableLog file output failed"};
        }

        mCurrentSize += recordBytes;
        mLastWriteDay = currentDay;
    }
    catch (...)
    {
        resetStream();
        throw;
    }
}

void FileOutput::flush()
{
    if (!mStream.is_open())
    {
        return;
    }

    try
    {
        mStream.flush();
        if (!mStream.good())
        {
            throw std::ios_base::failure{"SableLog file flush failed"};
        }
    }
    catch (...)
    {
        resetStream();
        throw;
    }
}

void FileOutput::openActiveFile()
{
    mStream.clear();
    mStream.open(mActivePath, std::ios::binary | std::ios::out | std::ios::app);

    if (!mStream.is_open() || !mStream.good())
    {
        throw std::ios_base::failure{"SableLog could not open the active log file"};
    }

    std::error_code error;
    const auto size = std::filesystem::file_size(mActivePath, error);
    if (error)
    {
        mStream.close();
        throw std::filesystem::filesystem_error{
            "SableLog could not determine the active log file size", mActivePath, error};
    }

    mCurrentSize = size;

    if (mCurrentSize == 0U)
    {
        mLastWriteDay = currentUtcDay();
        return;
    }

    const auto lastWriteTime = std::filesystem::last_write_time(mActivePath, error);
    if (error)
    {
        mStream.close();
        throw std::filesystem::filesystem_error{
            "SableLog could not determine the active log file date", mActivePath, error};
    }

    mLastWriteDay = toUtcDay(lastWriteTime);
}

void FileOutput::resetStream() noexcept
{
    if (mStream.is_open())
    {
        mStream.close();
    }

    mStream.clear();
}

void FileOutput::rotateIfNeeded(std::uint64_t recordBytes, std::chrono::sys_days currentDay)
{
    if (mCurrentSize == 0U)
    {
        return;
    }

    const auto shouldRotateForCalendar =
        crossedCalendarBoundary(mConfig.calendarRotation, mLastWriteDay, currentDay);
    const auto shouldRotateForSize =
        mCurrentSize >= mConfig.maxFileBytes ||
        recordBytes > mConfig.maxFileBytes - mCurrentSize;

    if (shouldRotateForCalendar || shouldRotateForSize)
    {
        rotate();
    }
}

void FileOutput::rotate()
{
    mStream.flush();
    if (!mStream.good())
    {
        throw std::ios_base::failure{"SableLog could not flush the active log file before rotation"};
    }

    mStream.close();
    if (mStream.fail())
    {
        throw std::ios_base::failure{"SableLog could not close the active log file before rotation"};
    }

    const auto archivePath = nextArchivePath(formatUtcDate(mLastWriteDay));
    std::error_code error;
    std::filesystem::rename(mActivePath, archivePath, error);
    if (error)
    {
        throw std::filesystem::filesystem_error{
            "SableLog could not rotate the active log file", mActivePath, archivePath, error};
    }

    mCurrentSize = 0U;
    openActiveFile();
    removeExpiredArchives();
}

void FileOutput::removeExpiredArchives() noexcept
{
    try
    {
        if (mConfig.retentionDays == 0U)
        {
            return;
        }

        using FileDuration = std::filesystem::file_time_type::duration;
        using Hours = std::chrono::hours;
        using FloatingHours = std::chrono::duration<long double, Hours::period>;

        const auto retentionHours = static_cast<std::uint64_t>(mConfig.retentionDays) * 24U;
        const auto maximumHours =
            std::chrono::duration_cast<FloatingHours>(FileDuration::max()).count();

        if (maximumHours <= 0.0L || static_cast<long double>(retentionHours) > maximumHours)
        {
            return;
        }

        const auto retention = std::chrono::duration_cast<FileDuration>(
            Hours{static_cast<Hours::rep>(retentionHours)});
        const auto now = std::filesystem::file_time_type::clock::now();

        std::error_code error;
        std::filesystem::directory_iterator iterator{
            mConfig.directory, std::filesystem::directory_options::skip_permission_denied, error};
        const std::filesystem::directory_iterator end;

        while (!error && iterator != end)
        {
            const auto path = iterator->path();
            const auto fileName = path.filename();

            std::error_code entryError;
            const auto isRegularFile = iterator->is_regular_file(entryError);
            const auto isArchive = FileArchiveName::matches(fileName, mConfig.baseName);

            if (!entryError && isRegularFile && isArchive)
            {
                const auto lastWriteTime = iterator->last_write_time(entryError);
                if (!entryError && lastWriteTime < now && now - lastWriteTime > retention)
                {
                    std::filesystem::remove(path, entryError);
                }
            }

            iterator.increment(error);
        }
    }
    catch (...)
    {
        // Retention is best-effort and must not make an otherwise usable output fail.
    }
}

std::filesystem::path FileOutput::nextArchivePath(std::string_view archiveDate)
{
    if (mRotationDate != archiveDate)
    {
        mRotationDate = archiveDate;
        mRotationSequence = 0U;
    }

    for (;;)
    {
        const auto sequence = mRotationSequence;
        const auto fileName = FileArchiveName::make(mConfig.baseName, archiveDate, sequence);
        const auto candidate = mConfig.directory / fileName;

        std::error_code error;
        const auto exists = std::filesystem::exists(candidate, error);
        if (error)
        {
            throw std::filesystem::filesystem_error{
                "SableLog could not check a rotated log file name", candidate, error};
        }
        if (!exists)
        {
            return candidate;
        }
        if (mRotationSequence == std::numeric_limits<std::uint64_t>::max())
        {
            throw std::overflow_error{"SableLog exhausted the rotated log file sequence"};
        }
        ++mRotationSequence;
    }
}

} // namespace sablelog::detail
