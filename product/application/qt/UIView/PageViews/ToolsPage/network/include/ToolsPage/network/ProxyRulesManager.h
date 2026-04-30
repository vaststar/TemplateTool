#pragma once

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QtQml>

class QTcpSocket;

/**
 * @brief Manages all proxy rule sets (mock, breakpoint, blacklist, map local/remote, throttle).
 *
 * Stores rules as QJsonArrays and syncs them to the mitmproxy addon via TCP
 * through a sendCommand callback.
 */
class ProxyRulesManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

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

    // ── URL Pattern Testing ──
    Q_INVOKABLE QString testUrlPattern(const QString& pattern, const QString& testUrl);

    /// Send all current rules to the addon (after reconnect).
    void sendAllRules();

    // Accessors for internal arrays (used by controller for initial sync)
    const QJsonArray& mockRules() const { return m_mockRules; }
    const QJsonArray& breakpointRules() const { return m_breakpointRules; }
    const QJsonArray& blacklistRules() const { return m_blacklistRules; }

private:
    void sendCommand(const QJsonObject& cmd);

    SendCommandFn m_sendCommandFn;

    QJsonArray m_mockRules;
    QJsonArray m_breakpointRules;
    QJsonArray m_blacklistRules;
    QJsonArray m_mapLocalRules;
    QJsonArray m_mapRemoteRules;
};
