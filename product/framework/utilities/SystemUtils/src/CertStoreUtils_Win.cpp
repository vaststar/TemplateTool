#include "CertStoreUtils_Win.h"

#include <fstream>
#include <vector>

#include <windows.h>
#include <wincrypt.h>

#pragma comment(lib, "crypt32.lib")

namespace ucf::utilities {

// ────────────────────────────────────────────────────────────
//  Helpers
// ────────────────────────────────────────────────────────────

static std::vector<char> readFileBytes(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        return {};
    }
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

/// Try to create a CERT_CONTEXT from raw file data (handles both PEM and DER).
static PCCERT_CONTEXT parseCertData(const std::vector<char>& data)
{
    if (data.empty())
    {
        return nullptr;
    }

    // Try DER first
    PCCERT_CONTEXT ctx = CertCreateCertificateContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        reinterpret_cast<const BYTE*>(data.data()),
        static_cast<DWORD>(data.size()));
    if (ctx)
    {
        return ctx;
    }

    // Likely PEM — decode base64 to DER
    DWORD derLen = 0;
    if (!CryptStringToBinaryA(
            data.data(), static_cast<DWORD>(data.size()),
            CRYPT_STRING_BASE64HEADER, nullptr, &derLen, nullptr, nullptr))
    {
        return nullptr;
    }

    std::vector<BYTE> derBuf(derLen);
    if (!CryptStringToBinaryA(
            data.data(), static_cast<DWORD>(data.size()),
            CRYPT_STRING_BASE64HEADER, derBuf.data(), &derLen, nullptr, nullptr))
    {
        return nullptr;
    }

    return CertCreateCertificateContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        derBuf.data(), derLen);
}

// ────────────────────────────────────────────────────────────
//  Public API
// ────────────────────────────────────────────────────────────

CertTrustStatus CertStoreUtils_Win::checkTrustStatus(const std::string& certPath)
{
    auto fileData = readFileBytes(certPath);
    if (fileData.empty())
    {
        return CertTrustStatus::FileNotFound;
    }

    PCCERT_CONTEXT pCertCtx = parseCertData(fileData);
    if (!pCertCtx)
    {
        return CertTrustStatus::Unknown; // parse failure
    }

    bool found = false;
    const wchar_t* storeName = L"Root";
    DWORD storeFlags[] = {
        CERT_SYSTEM_STORE_CURRENT_USER,
        CERT_SYSTEM_STORE_LOCAL_MACHINE
    };

    for (int i = 0; i < 2 && !found; ++i)
    {
        HCERTSTORE hStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM, 0, 0,
            storeFlags[i] | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG,
            storeName);

        if (hStore)
        {
            PCCERT_CONTEXT pFound = CertFindCertificateInStore(
                hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
                CERT_FIND_EXISTING, pCertCtx, nullptr);

            if (pFound)
            {
                found = true;
                CertFreeCertificateContext(pFound);
            }
            CertCloseStore(hStore, 0);
        }
    }

    CertFreeCertificateContext(pCertCtx);
    return found ? CertTrustStatus::Trusted : CertTrustStatus::NotTrusted;
}

CertInstallResult CertStoreUtils_Win::installToTrustStore(const std::string& certPath)
{
    auto fileData = readFileBytes(certPath);
    if (fileData.empty())
    {
        return CertInstallResult::FileNotFound;
    }

    PCCERT_CONTEXT pCertCtx = parseCertData(fileData);
    if (!pCertCtx)
    {
        return CertInstallResult::ParseError;
    }

    // Open current-user Root store (triggers a confirmation dialog)
    HCERTSTORE hStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM, 0, 0,
        CERT_SYSTEM_STORE_CURRENT_USER,
        L"Root");

    if (!hStore)
    {
        CertFreeCertificateContext(pCertCtx);
        return CertInstallResult::Failed;
    }

    BOOL ok = CertAddCertificateContextToStore(
        hStore, pCertCtx, CERT_STORE_ADD_REPLACE_EXISTING, nullptr);

    DWORD lastErr = ok ? 0 : GetLastError();
    CertCloseStore(hStore, 0);
    CertFreeCertificateContext(pCertCtx);

    if (ok)
    {
        return CertInstallResult::Success;
    }

    // ERROR_CANCELLED is returned when user clicks "No" on the security dialog
    if (lastErr == ERROR_CANCELLED)
    {
        return CertInstallResult::UserCancelled;
    }

    return CertInstallResult::Failed;
}

} // namespace ucf::utilities
