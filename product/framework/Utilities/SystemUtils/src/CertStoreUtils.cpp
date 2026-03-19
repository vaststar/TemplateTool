#include <ucf/Utilities/SystemUtils/CertStoreUtils.h>

#if defined(_WIN32)
#include "CertStoreUtils_Win.h"
#elif defined(__APPLE__)
#include "CertStoreUtils_Mac.h"
#elif defined(__linux__)
#include "CertStoreUtils_Linux.h"
#endif

namespace ucf::utilities {

CertTrustStatus CertStoreUtils::checkTrustStatus(const std::string& certPath)
{
#if defined(_WIN32)
    return CertStoreUtils_Win::checkTrustStatus(certPath);
#elif defined(__APPLE__)
    return CertStoreUtils_Mac::checkTrustStatus(certPath);
#elif defined(__linux__)
    return CertStoreUtils_Linux::checkTrustStatus(certPath);
#else
    return CertTrustStatus::Unknown;
#endif
}

CertInstallResult CertStoreUtils::installToTrustStore(const std::string& certPath)
{
#if defined(_WIN32)
    return CertStoreUtils_Win::installToTrustStore(certPath);
#elif defined(__APPLE__)
    return CertStoreUtils_Mac::installToTrustStore(certPath);
#elif defined(__linux__)
    return CertStoreUtils_Linux::installToTrustStore(certPath);
#else
    return CertInstallResult::Failed;
#endif
}

} // namespace ucf::utilities
