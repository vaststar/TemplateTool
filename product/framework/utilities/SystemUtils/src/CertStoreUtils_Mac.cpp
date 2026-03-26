#include "CertStoreUtils_Mac.h"

#include <cstdlib>
#include <sys/stat.h>

namespace ucf::utilities {

static bool fileExists(const std::string& path)
{
    struct stat st{};
    return stat(path.c_str(), &st) == 0;
}

CertTrustStatus CertStoreUtils_Mac::checkTrustStatus(const std::string& certPath)
{
    if (!fileExists(certPath))
    {
        return CertTrustStatus::FileNotFound;
    }

    // security verify-cert returns 0 if the certificate is trusted
    std::string cmd = "security verify-cert -c \"" + certPath + "\" >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    return (ret == 0) ? CertTrustStatus::Trusted : CertTrustStatus::NotTrusted;
}

CertInstallResult CertStoreUtils_Mac::installToTrustStore(const std::string& certPath)
{
    if (!fileExists(certPath))
    {
        return CertInstallResult::FileNotFound;
    }

    // Add to user login keychain as a trusted root.
    // This triggers a Keychain password prompt (user interaction required).
    std::string cmd = "security add-trusted-cert -r trustRoot"
                      " -k ~/Library/Keychains/login.keychain-db \""
                      + certPath + "\" 2>/dev/null";

    int ret = std::system(cmd.c_str());
    if (ret == 0)
    {
        return CertInstallResult::Success;
    }

    // Non-zero could be user cancel or other error; macOS doesn't distinguish clearly
    return CertInstallResult::Failed;
}

} // namespace ucf::utilities
