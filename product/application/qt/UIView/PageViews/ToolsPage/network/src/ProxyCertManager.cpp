#include "PageViews/ToolsPage/network/include/ProxyCertManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QSslCertificate>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")

// Helper: parse a certificate file that may be PEM or DER encoded
static PCCERT_CONTEXT parseCertFile(const QByteArray& fileData)
{
    if (fileData.isEmpty())
        return nullptr;

    // Try DER first
    PCCERT_CONTEXT ctx = CertCreateCertificateContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        reinterpret_cast<const BYTE*>(fileData.constData()),
        static_cast<DWORD>(fileData.size()));
    if (ctx)
        return ctx;

    // Likely PEM — decode base64 to DER via CryptStringToBinaryA
    DWORD derLen = 0;
    if (!CryptStringToBinaryA(fileData.constData(), static_cast<DWORD>(fileData.size()),
                              CRYPT_STRING_BASE64HEADER, nullptr, &derLen, nullptr, nullptr))
        return nullptr;

    QByteArray derBuf(static_cast<int>(derLen), '\0');
    if (!CryptStringToBinaryA(fileData.constData(), static_cast<DWORD>(fileData.size()),
                              CRYPT_STRING_BASE64HEADER,
                              reinterpret_cast<BYTE*>(derBuf.data()), &derLen, nullptr, nullptr))
        return nullptr;

    return CertCreateCertificateContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        reinterpret_cast<const BYTE*>(derBuf.constData()),
        derLen);
}
#endif

ProxyCertManager::ProxyCertManager(QObject* parent)
    : QObject(parent)
{
}

bool ProxyCertManager::isCertInstalling() const { return m_certInstalling; }

QString ProxyCertManager::getCACertPath() const
{
    QString home = QDir::homePath();
#if defined(Q_OS_MACOS)
    return home + "/.mitmproxy/mitmproxy-ca-cert.pem";
#elif defined(Q_OS_WIN)
    // Use .cer (DER encoded) on Windows — easy to import via CryptoAPI
    return home + "/.mitmproxy/mitmproxy-ca-cert.cer";
#else
    return home + "/.mitmproxy/mitmproxy-ca-cert.pem";
#endif
}

bool ProxyCertManager::isCACertInstalled() const
{
    QString certPath = getCACertPath();
    if (!QFile::exists(certPath))
        return false;

#if defined(Q_OS_MACOS)
    QProcess verifyProc;
    verifyProc.start(QStringLiteral("security"),
                     {"verify-cert", "-c", certPath});
    verifyProc.waitForFinished(3000);
    return (verifyProc.exitCode() == 0);
#elif defined(Q_OS_WIN)
    // Read the .cer file and extract the subject name for lookup
    QFile certFile(certPath);
    if (!certFile.open(QIODevice::ReadOnly))
        return false;
    QByteArray certData = certFile.readAll();
    certFile.close();

    if (certData.isEmpty())
        return false;

    // Parse the certificate (handles both PEM and DER)
    PCCERT_CONTEXT pCertCtx = parseCertFile(certData);
    if (!pCertCtx)
        return false;

    bool found = false;

    // Check current-user Root store first, then machine Root store
    const wchar_t* storeNames[] = { L"Root", L"Root" };
    DWORD storeFlags[] = {
        CERT_SYSTEM_STORE_CURRENT_USER,
        CERT_SYSTEM_STORE_LOCAL_MACHINE
    };

    for (int i = 0; i < 2 && !found; ++i) {
        HCERTSTORE hStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM, 0, 0,
            storeFlags[i] | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG,
            storeNames[i]);

        if (hStore) {
            // Search by the exact certificate content (thumbprint match)
            PCCERT_CONTEXT pFound = CertFindCertificateInStore(
                hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
                CERT_FIND_EXISTING, pCertCtx, nullptr);

            if (pFound) {
                found = true;
                CertFreeCertificateContext(pFound);
            }
            CertCloseStore(hStore, 0);
        }
    }

    CertFreeCertificateContext(pCertCtx);
    return found;
#else
    return true;
#endif
}

void ProxyCertManager::revealCACertInFolder()
{
    QString certPath = getCACertPath();
    QString folder = QFileInfo(certPath).absolutePath();

    if (!QDir(folder).exists()) {
        emit statusMessage(tr("Certificate folder does not exist yet. Start the proxy first to generate the CA certificate."));
        return;
    }

#if defined(Q_OS_MACOS)
    if (QFile::exists(certPath))
        QProcess::startDetached(QStringLiteral("open"), {"-R", certPath});
    else
        QProcess::startDetached(QStringLiteral("open"), {folder});
#elif defined(Q_OS_WIN)
    QProcess::startDetached(QStringLiteral("explorer"), {"/select,", QDir::toNativeSeparators(certPath)});
#else
    QProcess::startDetached(QStringLiteral("xdg-open"), {folder});
#endif
    emit statusMessage(tr("Certificate folder: %1").arg(folder));
}

void ProxyCertManager::installCACert()
{
    QString certPath = getCACertPath();

    if (!QFile::exists(certPath)) {
        QString mitmDir = QDir::homePath() + "/.mitmproxy";
        if (!QDir(mitmDir).exists())
            emit statusMessage(tr("Please start the proxy first. The CA certificate is generated on first launch."));
        else
            emit statusMessage(tr("CA certificate not found at %1. Try restarting the proxy.").arg(certPath));
        return;
    }

    if (m_certInstalling) return;

    m_certInstalling = true;
    emit certInstallingChanged();
    emit statusMessage(tr("Installing certificate... A system password dialog will appear."));

#if defined(Q_OS_MACOS)
    QString loginKeychain = QDir::homePath() + "/Library/Keychains/login.keychain-db";

    // Step 1: Import (synchronous, fast, no dialog)
    {
        QProcess importProc;
        importProc.start(QStringLiteral("security"),
                         {"import", certPath, "-k", loginKeychain, "-t", "cert"});
        importProc.waitForFinished(5000);
    }

    // Step 2: Trust (async — shows macOS native password dialog)
    m_certProcess = new QProcess(this);

    connect(m_certProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus) {
        m_certInstalling = false;
        emit certInstallingChanged();

        bool trusted = isCACertInstalled();

        if (trusted) {
            emit statusMessage(tr("✅ CA Certificate installed and trusted successfully! HTTPS capture is ready."));
            emit caCertInstalledChanged();
        } else if (exitCode != 0) {
            emit statusMessage(tr("Certificate trust was not set. You may have cancelled the dialog. Try again."));
        } else {
            emit statusMessage(tr("Certificate imported but trust verification failed. Try clicking Refresh Status."));
            emit caCertInstalledChanged();
        }

        m_certProcess->deleteLater();
        m_certProcess = nullptr;
    });

    m_certProcess->start(QStringLiteral("security"),
                         {"add-trusted-cert", "-r", "trustRoot", certPath});

#elif defined(Q_OS_WIN)
    // Use Windows CryptoAPI directly — install to Current User Root store (no admin needed)
    QFile certFile(certPath);
    if (!certFile.open(QIODevice::ReadOnly)) {
        m_certInstalling = false;
        emit certInstallingChanged();
        emit statusMessage(tr("Failed to read certificate file: %1").arg(certPath));
        return;
    }
    QByteArray certData = certFile.readAll();
    certFile.close();

    // Parse the certificate (handles both PEM and DER)
    PCCERT_CONTEXT pCertCtx = parseCertFile(certData);

    if (!pCertCtx) {
        m_certInstalling = false;
        emit certInstallingChanged();
        emit statusMessage(tr("Failed to parse certificate. The file may be corrupted."));
        return;
    }

    // Open the Current User "Root" (Trusted Root Certification Authorities) store
    HCERTSTORE hStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM, 0, 0,
        CERT_SYSTEM_STORE_CURRENT_USER,
        L"Root");

    bool success = false;
    if (hStore) {
        // Note: Windows will show a security dialog asking the user to confirm
        // adding a cert to the trusted root store — this is expected and does NOT need admin.
        success = CertAddCertificateContextToStore(
            hStore, pCertCtx, CERT_STORE_ADD_REPLACE_EXISTING, nullptr);
        CertCloseStore(hStore, 0);
    }

    CertFreeCertificateContext(pCertCtx);

    m_certInstalling = false;
    emit certInstallingChanged();

    if (success) {
        emit statusMessage(tr("✅ CA Certificate installed to Windows cert store! HTTPS capture is ready."));
        emit caCertInstalledChanged();
    } else {
        DWORD err = GetLastError();
        emit statusMessage(tr("Certificate installation failed (error code: %1). "
                              "If you cancelled the security dialog, please try again and click Yes.")
                           .arg(err));
    }

#elif defined(Q_OS_LINUX)
    QProcess::execute(QStringLiteral("sudo"),
        {"cp", certPath, "/usr/local/share/ca-certificates/mitmproxy-ca-cert.crt"});
    int ret = QProcess::execute(QStringLiteral("sudo"), {"update-ca-certificates"});
    m_certInstalling = false;
    emit certInstallingChanged();
    if (ret == 0) {
        emit statusMessage(tr("✅ CA Certificate installed!"));
        emit caCertInstalledChanged();
    } else {
        emit statusMessage(tr("Certificate installation failed."));
    }
#endif
}
