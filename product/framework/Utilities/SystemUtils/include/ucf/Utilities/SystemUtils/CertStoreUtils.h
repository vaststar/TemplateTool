#pragma once

#include <string>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

/// Result of checking whether a certificate is trusted by the OS.
enum class CertTrustStatus
{
    Unknown,       ///< Unable to determine
    FileNotFound,  ///< The certificate file does not exist
    NotTrusted,    ///< File exists but is NOT in the system trust store
    Trusted        ///< Certificate IS in the system trust store
};

/// Result of attempting to install a certificate into the trust store.
enum class CertInstallResult
{
    Success,        ///< Certificate installed successfully
    FileNotFound,   ///< Certificate file does not exist
    ParseError,     ///< Certificate file could not be parsed (bad format)
    UserCancelled,  ///< User declined the OS security prompt
    Failed          ///< Other / unknown failure
};

/// Cross-platform certificate trust-store operations.
///
/// @note installToTrustStore() may trigger an OS-level security prompt
/// requiring user confirmation (Windows) or password input (macOS / Linux).
class Utilities_EXPORT CertStoreUtils final
{
public:
    /// Check whether a CA certificate file is trusted by the system.
    static CertTrustStatus checkTrustStatus(const std::string& certPath);

    /// Install a CA certificate into the system trust store.
    ///
    /// @warning This may trigger an OS security dialog:
    ///   - Windows: "You are about to install a certificate" confirmation
    ///   - macOS:   Keychain password prompt
    ///   - Linux:   sudo password prompt (via update-ca-certificates)
    static CertInstallResult installToTrustStore(const std::string& certPath);
};

} // namespace ucf::utilities
