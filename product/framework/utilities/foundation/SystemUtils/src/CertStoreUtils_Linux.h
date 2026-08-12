#pragma once

#include <string>

#include <ucf/utilities/SystemUtils/CertStoreUtils.h>

namespace ucf::utilities {

class CertStoreUtils_Linux final
{
public:
    static CertTrustStatus checkTrustStatus(const std::string& certPath);
    static CertInstallResult installToTrustStore(const std::string& certPath);
};

} // namespace ucf::utilities
