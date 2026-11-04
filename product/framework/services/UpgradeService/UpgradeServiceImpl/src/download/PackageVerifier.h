#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace ucf::service::upgrade {

enum class VerificationStatus
{
    Passed,
    Failed,
    Cancelled
};

struct VerificationResult
{
    VerificationStatus status;
    std::string message;
};

/// Verifies the file synchronously. The cancellation predicate is called while
/// hashing and is never retained after this function returns.
[[nodiscard]] VerificationResult verifyPackageFile(
    std::string_view filePath, std::string_view expectedSha256, const std::function<bool()>& isCancelled);

} // namespace ucf::service::upgrade
