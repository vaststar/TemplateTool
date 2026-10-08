#pragma once

#include <functional>

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>
#include <QtQml>

/**
 * @brief Manages all proxy rule sets (mock, breakpoint, rewrite, blacklist, map local/remote, throttle).
 *
 * Stores rules as QJsonArrays and syncs them to the mitmproxy addon via TCP
 * through a sendCommand callback.
 */
class ProxyRulesManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantList rewriteRules READ getRewriteRules NOTIFY rewriteRulesChanged)
    Q_PROPERTY(QString rewriteError READ getRewriteError NOTIFY rewriteErrorChanged)
    Q_PROPERTY(bool throttleEnabled READ throttleEnabled NOTIFY throttleChanged)
    Q_PROPERTY(int downloadKbps READ downloadKbps NOTIFY throttleChanged)
    Q_PROPERTY(int uploadKbps READ uploadKbps NOTIFY throttleChanged)
    Q_PROPERTY(QString configSyncState READ getConfigSyncState NOTIFY configSyncChanged)
    Q_PROPERTY(QString configSyncError READ getConfigSyncError NOTIFY configSyncChanged)
    Q_PROPERTY(QString configRevision READ getConfigRevision NOTIFY configSyncChanged)

public:
    explicit ProxyRulesManager(QObject* parent = nullptr);

    /// Set the callback used to send TCP commands to the addon.
    using SendCommandFn = std::function<void(const QJsonObject&)>;
    void setSendCommandFn(SendCommandFn fn);

    // ── Mock Rules ──
    Q_INVOKABLE void addMockRule(const QString& urlPattern, int statusCode,
                                  const QString& contentType, const QString& body,
                                  const QString& headers = QString());
    Q_INVOKABLE void removeMockRule(int index);
    Q_INVOKABLE void clearMockRules();
    Q_INVOKABLE QVariantList getMockRules() const;

    // ── Rewrite Rules ──
    Q_INVOKABLE bool saveRewriteRule(int index, const QVariantMap& rule);
    Q_INVOKABLE void removeRewriteRule(int index);
    Q_INVOKABLE void clearRewriteRules();
    Q_INVOKABLE void setRewriteRuleEnabled(int index, bool enabled);
    Q_INVOKABLE void moveRewriteRule(int from, int to);
    QVariantList getRewriteRules() const;
    QString getRewriteError() const { return m_rewriteError; }

    // ── Breakpoint Rules ──
    Q_INVOKABLE void addBreakpointRule(const QString& urlPattern, const QString& method);
    Q_INVOKABLE void removeBreakpointRule(int index);
    Q_INVOKABLE void clearBreakpointRules();
    Q_INVOKABLE QVariantList getBreakpointRules() const;

    // ── Blacklist ──
    Q_INVOKABLE void addBlacklistRule(const QString& urlPattern);
    Q_INVOKABLE void removeBlacklistRule(int index);
    Q_INVOKABLE QVariantList getBlacklistRules() const;

    // ── Map Local ──
    Q_INVOKABLE void addMapLocalRule(const QString& urlPattern, const QString& localPath);
    Q_INVOKABLE void removeMapLocalRule(int index);
    Q_INVOKABLE QVariantList getMapLocalRules() const;

    // ── Map Remote ──
    Q_INVOKABLE void addMapRemoteRule(const QString& srcPattern, const QString& destUrl);
    Q_INVOKABLE void removeMapRemoteRule(int index);
    Q_INVOKABLE QVariantList getMapRemoteRules() const;

    // ── Throttle ──
    Q_INVOKABLE void setThrottle(bool enabled, int downloadKBps, int uploadKBps);
    bool throttleEnabled() const { return m_throttleEnabled; }
    int downloadKbps() const { return m_downloadKbps; }
    int uploadKbps() const { return m_uploadKbps; }

    // ── Configuration Synchronization ──
    void setInterceptEnabled(bool enabled);
    void setAddonConnected(bool connected);
    void handleConfigResult(const QString& sessionId, const QString& revision,
                            bool accepted, const QString& message);
    Q_INVOKABLE void retryConfigSync();
    QString getConfigSyncState() const { return m_configSyncState; }
    QString getConfigSyncError() const { return m_configSyncError; }
    QString getConfigRevision() const { return QString::number(m_configRevision); }

    // ── Bypass Hosts (passthrough / no MITM) ──
    Q_INVOKABLE void setBypassHosts(const QStringList& hostPatterns);
    Q_INVOKABLE QStringList getBypassHosts() const;

    // ── URL Pattern Testing ──
    Q_INVOKABLE QString testUrlPattern(const QString& pattern, const QString& testUrl);

    /// Retry the complete configuration. Reconnects use setAddonConnected(true).
    void sendAllRules();

    // Accessors for internal arrays (used by controller for initial sync)
    const QJsonArray& mockRules() const { return m_mockRules; }
    const QJsonArray& breakpointRules() const { return m_breakpointRules; }
    const QJsonArray& blacklistRules() const { return m_blacklistRules; }

signals:
    void rewriteRulesChanged();
    void rewriteErrorChanged();
    void throttleChanged();
    void configSyncChanged();

private:
    void sendCommand(const QJsonObject& cmd);
    bool validateRewriteRule(const QJsonObject& input, QJsonObject& result, QString& error) const;
    void setRewriteError(const QString& error);
    void publishRewriteRules();
    QJsonObject configurationSnapshot() const;
    void synchronizeConfiguration();
    void setConfigSyncState(const QString& state, const QString& error = QString());

    SendCommandFn m_sendCommandFn;

    QJsonArray m_mockRules;
    QJsonArray m_rewriteRules;
    QString m_rewriteError;
    QJsonArray m_breakpointRules;
    QJsonArray m_blacklistRules;
    QJsonArray m_mapLocalRules;
    QJsonArray m_mapRemoteRules;
    QStringList m_bypassHosts;
    bool m_throttleEnabled = false;
    int m_downloadKbps = 0;
    int m_uploadKbps = 0;
    bool m_interceptEnabled = false;
    bool m_addonConnected = false;
    QString m_configSessionId;
    quint64 m_configRevision = 0;
    QString m_configSyncState = QStringLiteral("pending");
    QString m_configSyncError;
    QTimer m_configAckTimer;
};
