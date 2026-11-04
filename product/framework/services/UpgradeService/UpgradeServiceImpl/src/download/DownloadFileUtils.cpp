#include "DownloadFileUtils.h"
#include "../UpgradeServiceLogger.h"

#include <ucf/utilities/FilePathUtils/FilePathUtils.h>

#include <exception>
#include <limits>
#include <system_error>

namespace ucf::service::upgrade {

bool hasSufficientDownloadSpace(const std::filesystem::path& directory, std::int64_t requiredBytes)
{
    if (requiredBytes <= 0)
    {
        UPGRADE_LOG_ERROR("Invalid package size for disk space check: " << requiredBytes);
        return false;
    }
    const auto bytes = static_cast<std::uintmax_t>(requiredBytes);
    constexpr auto kMaxBytes = (std::numeric_limits<std::uintmax_t>::max)();

    try
    {
        ucf::utilities::FilePathUtils::createDirectoriesUtf8(ucf::utilities::FilePathUtils::utf8FromPath(directory));
        const auto spaceInfo = std::filesystem::space(directory);
        if (spaceInfo.available == kMaxBytes)
        {
            UPGRADE_LOG_ERROR("Available disk space could not be determined");
            return false;
        }
        // Installation space is checked by the system installer on its target volume.
        return spaceInfo.available >= bytes;
    }
    catch (const std::exception& ex)
    {
        UPGRADE_LOG_ERROR("Failed to check disk space: " << ex.what());
        return false;
    }
}

bool isReusableInstallerFile(const std::filesystem::path& filePath, std::int64_t expectedBytes)
{
    std::error_code ec;
    const auto status = std::filesystem::symlink_status(filePath, ec);
    return !ec && std::filesystem::is_regular_file(status) && expectedBytes > 0 &&
           std::filesystem::file_size(filePath, ec) == static_cast<std::uintmax_t>(expectedBytes) && !ec;
}

std::optional<std::string> validateDownloadedFileSize(const std::filesystem::path& filePath, std::int64_t expectedBytes)
{
    std::error_code ec;
    const auto actualSize = std::filesystem::file_size(filePath, ec);
    if (ec)
    {
        return "Downloaded file is unavailable: " + ec.message();
    }
    if (expectedBytes > 0 && actualSize != static_cast<std::uintmax_t>(expectedBytes))
    {
        return "Downloaded size " + std::to_string(actualSize) + " differs from expected " +
               std::to_string(expectedBytes);
    }
    if (expectedBytes <= 0 && actualSize == 0)
    {
        return "Downloaded file is empty";
    }
    return std::nullopt;
}

void removeDownloadFile(const std::filesystem::path& filePath, std::string_view failureContext)
{
    std::error_code ec;
    std::filesystem::remove(filePath, ec);
    if (ec)
    {
        UPGRADE_LOG_WARN(failureContext << ": " << ec.message());
    }
}

} // namespace ucf::service::upgrade
