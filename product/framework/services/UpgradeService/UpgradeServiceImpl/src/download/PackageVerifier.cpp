#include "PackageVerifier.h"
#include "../UpgradeServiceLogger.h"

#include <ucf/utilities/FilePathUtils/FilePathUtils.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>

#include <openssl/evp.h>

namespace ucf::service::upgrade {
namespace {

std::optional<std::string> computeSha256(
    const std::filesystem::path& filePath, const std::function<bool()>& isCancelled)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error(
            "Cannot open file for SHA-256: " + ucf::utilities::FilePathUtils::utf8FromPath(filePath));
    }

    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!ctx || EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1)
    {
        throw std::runtime_error("SHA-256 initialization failed");
    }

    char buffer[8192];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
    {
        if (isCancelled())
        {
            return std::nullopt;
        }
        if (EVP_DigestUpdate(ctx.get(), buffer, static_cast<std::size_t>(file.gcount())) != 1)
        {
            throw std::runtime_error("SHA-256 update failed");
        }
    }
    if (file.bad())
    {
        throw std::runtime_error(
            "Failed to read file for SHA-256: " + ucf::utilities::FilePathUtils::utf8FromPath(filePath));
    }
    if (isCancelled())
    {
        return std::nullopt;
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;
    if (EVP_DigestFinal_ex(ctx.get(), hash, &hashLen) != 1)
    {
        throw std::runtime_error("SHA-256 finalization failed");
    }

    std::ostringstream oss;
    for (unsigned int i = 0; i < hashLen; ++i)
    {
        oss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(hash[i]);
    }
    return oss.str();
}

} // namespace

VerificationResult verifyPackageFile(
    std::string_view filePath, std::string_view expectedSha256, const std::function<bool()>& isCancelled)
{
    try
    {
        if (expectedSha256.empty())
        {
            const std::string message = "Expected SHA-256 is missing";
            UPGRADE_LOG_ERROR(message);
            return {VerificationStatus::Failed, message};
        }
        if (!ucf::utilities::FilePathUtils::existsUtf8(filePath))
        {
            return {VerificationStatus::Failed, "File not found: " + std::string(filePath)};
        }

        const auto actualHash = computeSha256(ucf::utilities::FilePathUtils::pathFromUtf8(filePath), isCancelled);
        if (!actualHash)
        {
            return {VerificationStatus::Cancelled, {}};
        }
        if (*actualHash != expectedSha256)
        {
            UPGRADE_LOG_ERROR("SHA-256 mismatch: expected=" << expectedSha256 << " actual=" << *actualHash);
            return {VerificationStatus::Failed,
                "SHA-256 mismatch: expected " + std::string(expectedSha256) + ", got " + *actualHash};
        }
        UPGRADE_LOG_INFO("Package verification passed");
        return {VerificationStatus::Passed, {}};
    }
    catch (const std::exception& ex)
    {
        UPGRADE_LOG_ERROR("Verification exception: " << ex.what());
        return {VerificationStatus::Failed, ex.what()};
    }
}

} // namespace ucf::service::upgrade
