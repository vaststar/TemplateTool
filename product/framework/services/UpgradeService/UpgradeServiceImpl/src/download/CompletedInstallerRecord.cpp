#include "CompletedInstallerRecord.h"
#include "../UpgradeServiceLogger.h"

#include <ucf/utilities/FilePathUtils/FilePathUtils.h>
#include <ucf/utilities/JsonUtils/JsonValue.h>

#include <algorithm>
#include <array>
#include <exception>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

namespace ucf::service::upgrade {
namespace {

constexpr auto kRecordFileName = "completed-installer.json";
constexpr auto kTemporaryRecordFileName = "completed-installer.json.tmp";
constexpr std::size_t kMaxRecordBytes = 4096;
constexpr std::string_view kInstallerPrefix = "installer-";

bool isSafeInstallerFileName(std::string_view fileName, model::PackageFormat format)
{
    const auto formatName = model::packageFormatName(format);
    const auto extension = "." + std::string(formatName);
    if (formatName.empty() || fileName.size() > 255 || fileName.size() <= kInstallerPrefix.size() + extension.size() ||
        !fileName.starts_with(kInstallerPrefix) || !fileName.ends_with(extension))
    {
        return false;
    }
    const auto identifier =
        fileName.substr(kInstallerPrefix.size(), fileName.size() - kInstallerPrefix.size() - extension.size());
    return std::all_of(identifier.begin(), identifier.end(), [](unsigned char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '-' ||
               ch == '_';
    });
}

} // namespace

bool sameInstallerContent(const model::PackageInfo& lhs, const model::PackageInfo& rhs) noexcept
{
    return lhs.sizeBytes > 0 && !model::packageFormatName(lhs.format).empty() && lhs.sha256.size() == 64 &&
           std::all_of(lhs.sha256.begin(), lhs.sha256.end(),
               [](unsigned char ch) {
        return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
    }) && lhs.sha256 == rhs.sha256 &&
           lhs.sizeBytes == rhs.sizeBytes && lhs.format == rhs.format;
}

std::optional<CompletedInstaller> readCompletedInstallerRecord(
    const std::filesystem::path& directory, const model::PackageInfo& expected)
{
    if (!sameInstallerContent(expected, expected))
    {
        return std::nullopt;
    }
    try
    {
        std::ifstream input(directory / kRecordFileName, std::ios::binary);
        if (!input)
        {
            return std::nullopt;
        }
        std::array<char, kMaxRecordBytes + 1> buffer{};
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto length = input.gcount();
        if (input.bad() || length <= 0 || length > static_cast<std::streamsize>(kMaxRecordBytes))
        {
            UPGRADE_LOG_DEBUG("Ignoring empty, unreadable or oversized installer record");
            return std::nullopt;
        }
        const auto parsed =
            ucf::utilities::JsonValue::parseEx(std::string(buffer.data(), static_cast<std::size_t>(length)));
        if (!parsed.ok() || !parsed.value.isObject())
        {
            UPGRADE_LOG_DEBUG("Ignoring malformed installer record");
            return std::nullopt;
        }
        const auto& record = parsed.value;
        const auto fileName = record.get("fileName").asString();
        const auto sha256 = record.get("sha256").asString();
        const auto format = record.get("format").asString();
        if (record.get("schemaVersion").dump() != "1" || !fileName ||
            !isSafeInstallerFileName(*fileName, expected.format) || !sha256 || *sha256 != expected.sha256 || !format ||
            *format != model::packageFormatName(expected.format) ||
            record.get("size").dump() != std::to_string(expected.sizeBytes))
        {
            UPGRADE_LOG_DEBUG("Installer record does not match the current manifest");
            return std::nullopt;
        }
        return CompletedInstaller{expected, directory / ucf::utilities::FilePathUtils::pathFromUtf8(*fileName)};
    }
    catch (const std::exception& ex)
    {
        UPGRADE_LOG_WARN("Could not read installer record: " << ex.what());
        return std::nullopt;
    }
}

void writeCompletedInstallerRecord(
    const std::filesystem::path& directory, const std::optional<CompletedInstaller>& installer)
{
    const auto recordPath = directory / kRecordFileName;
    const auto temporaryPath = directory / kTemporaryRecordFileName;
    std::error_code ec;
    if (!installer)
    {
        std::filesystem::remove(recordPath, ec);
        if (ec)
        {
            UPGRADE_LOG_WARN("Could not remove installer record: " << ec.message());
        }
        return;
    }
    try
    {
        const auto fileName = ucf::utilities::FilePathUtils::utf8FromPath(installer->filePath.filename());
        if (!sameInstallerContent(installer->packageInfo, installer->packageInfo) ||
            !isSafeInstallerFileName(fileName, installer->packageInfo.format) ||
            installer->filePath.lexically_normal().parent_path() != directory.lexically_normal())
        {
            UPGRADE_LOG_WARN("Refusing to save an invalid installer record");
            return;
        }
        auto record = ucf::utilities::JsonValue::object();
        record.set("schemaVersion", 1);
        record.set("fileName", fileName);
        record.set("sha256", installer->packageInfo.sha256);
        record.set("size", installer->packageInfo.sizeBytes);
        record.set("format", std::string(model::packageFormatName(installer->packageInfo.format)));
        const auto content = record.dump();
        std::ofstream output(temporaryPath, std::ios::binary | std::ios::trunc);
        if (!output)
        {
            UPGRADE_LOG_WARN("Could not create temporary installer record");
        }
        else
        {
            output.write(content.data(), static_cast<std::streamsize>(content.size()));
            output.close();
            if (!output)
            {
                UPGRADE_LOG_WARN("Could not finish writing temporary installer record");
            }
            else
            {
                std::filesystem::rename(temporaryPath, recordPath, ec);
#if defined(_WIN32)
                // Windows rename may not replace an existing file. A crash
                // between removal and rename can lose this disposable hint.
                if (ec)
                {
                    std::error_code existsError;
                    if (std::filesystem::exists(recordPath, existsError) && !existsError)
                    {
                        std::filesystem::remove(recordPath, ec);
                        if (!ec)
                        {
                            std::filesystem::rename(temporaryPath, recordPath, ec);
                        }
                    }
                }
#endif
                if (!ec)
                {
                    return;
                }
                UPGRADE_LOG_WARN("Could not replace installer record: " << ec.message());
            }
        }
    }
    catch (const std::exception& ex)
    {
        UPGRADE_LOG_WARN("Could not save installer record: " << ex.what());
    }
    ec.clear();
    std::filesystem::remove(temporaryPath, ec);
    if (ec)
    {
        UPGRADE_LOG_WARN("Could not remove temporary installer record: " << ec.message());
    }
}

} // namespace ucf::service::upgrade
