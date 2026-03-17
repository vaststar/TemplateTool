#pragma once

#include <QObject>
#include <QProcess>
#include <QtQml>

/**
 * @brief Manages mitmproxy CA certificate discovery, installation, and trust verification.
 */
class ProxyCertManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool caCertInstalled READ isCACertInstalled NOTIFY caCertInstalledChanged)
    Q_PROPERTY(bool certInstalling  READ isCertInstalling  NOTIFY certInstallingChanged)

public:
    explicit ProxyCertManager(QObject* parent = nullptr);

    bool isCertInstalling() const;

    Q_INVOKABLE bool    isCACertInstalled() const;
    Q_INVOKABLE QString getCACertPath() const;
    Q_INVOKABLE void    installCACert();
    Q_INVOKABLE void    revealCACertInFolder();

signals:
    void caCertInstalledChanged();
    void certInstallingChanged();
    void statusMessage(const QString& msg);

private:
    bool       m_certInstalling = false;
    QProcess*  m_certProcess = nullptr;
};
