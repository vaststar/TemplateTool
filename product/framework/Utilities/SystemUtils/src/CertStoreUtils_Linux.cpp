#include "CertStoreUtils_Linux.h"

#include <cstdlib>
#include <fstream>
#include <sys/stat.h>

namespace ucf::utilities {

static bool fileExists(const std::string& path)
{
    struct stat st{};
    return stat(path.c_str(), &st) == 0;
}

CertTrustStatus CertStoreUtils_Linux::checkTrustStatus(const std::string& certPath)
{
    if (!fileExists(certPath))
    {
        return CertTrustStatus::FileNotFound;
    }

    // Best-effort: check if the cert has been copied into the system trust dir.
    // This is a heuristic — the authoritative check would parse
    // /etc/ssl/certs/ca-certificates.crt, but filename-based check is simpler.
    const char* trustDir = "/usr/local/share/ca-certificates/";
    // Extract filename from path
    auto lastSlash = certPath.rfind('/');
    std::string filename = (lastSlash != std::string::npos)
                               ? certPath.substr(lastSlash + 1)
                               : certPath;

    // Replace extension with .crt (update-ca-certificates convention)
    auto dot = filename.rfind('.');
    if (dot != std::string::npos)
    {
        filename = filename.substr(0, dot) + ".crt";
    }

    if (fileExists(std::string(trustDir) + filename))
    {
        return CertTrustStatus::Trusted;
    }
    return CertTrustStatus::NotTrusted;
}

CertInstallResult CertStoreUtils_Linux::installToTrustStore(const std::string& certPath)
{
    if (!fileExists(certPath))
    {
        return CertInstallResult::FileNotFound;
    }

    // Extract filename for the target path
    auto lastSlash = certPath.rfind('/');
    std::string filename = (lastSlash != std::string::npos)
                               ? certPath.substr(lastSlash + 1)
                               : certPath;
    auto dot = filename.rfind('.');
    if (dot != std::string::npos)
    {
        filename = filename.substr(0, dot) + ".crt";
    }

    // Requires sudo — will prompt for password in a terminal context.
    // In a GUI application, consider using pkexec instead.
    std::string cmd = "sudo cp \"" + certPath
                      + "\" /usr/local/share/ca-certificates/" + filename
                      + " && sudo update-ca-certificates 2>/dev/null";

    int ret = std::system(cmd.c_str());
    return (ret == 0) ? CertInstallResult::Success : CertInstallResult::Failed;
}

} // namespace ucf::utilities
