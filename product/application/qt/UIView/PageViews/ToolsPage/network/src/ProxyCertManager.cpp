#include "PageViews/ToolsPage/network/include/ProxyCertManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

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
    return home + "/.mitmproxy/mitmproxy-ca-cert.p12";
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
    m_certProcess = new QProcess(this);
    connect(m_certProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus) {
        m_certInstalling = false;
        emit certInstallingChanged();
        if (exitCode == 0) {
            emit statusMessage(tr("✅ CA Certificate installed to Windows cert store!"));
            emit caCertInstalledChanged();
        } else {
            emit statusMessage(tr("Certificate installation failed (run as administrator?)"));
        }
        m_certProcess->deleteLater();
        m_certProcess = nullptr;
    });
    m_certProcess->start(QStringLiteral("certutil"), {"-addstore", "Root", certPath});

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
