#include "PageViews/ToolsPage/network/include/ProxyRulesManager.h"

#include <QRegularExpression>

ProxyRulesManager::ProxyRulesManager(QObject* parent)
    : QObject(parent)
{
}

void ProxyRulesManager::setSendCommandFn(SendCommandFn fn)
{
    m_sendCommandFn = std::move(fn);
}

void ProxyRulesManager::sendCommand(const QJsonObject& cmd)
{
    if (m_sendCommandFn)
        m_sendCommandFn(cmd);
}

// ====================== Mock Rules ======================

void ProxyRulesManager::addMockRule(const QString& urlPattern, int statusCode,
                                     const QString& contentType, const QString& body,
                                     const QString& headers)
{
    QJsonObject rule;
    rule["url_pattern"] = urlPattern;
    rule["status_code"] = statusCode;
    rule["content_type"] = contentType;
    rule["body"] = body;
    if (!headers.trimmed().isEmpty())
        rule["headers"] = headers.trimmed();
    m_mockRules.append(rule);

    QJsonObject cmd;
    cmd["type"] = QStringLiteral("update_mock_rules");
    cmd["rules"] = m_mockRules;
    sendCommand(cmd);
}

void ProxyRulesManager::removeMockRule(int index)
{
    if (index >= 0 && index < m_mockRules.size()) {
        m_mockRules.removeAt(index);
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_mock_rules");
        cmd["rules"] = m_mockRules;
        sendCommand(cmd);
    }
}

void ProxyRulesManager::clearMockRules()
{
    m_mockRules = QJsonArray();
    QJsonObject cmd;
    cmd["type"] = QStringLiteral("update_mock_rules");
    cmd["rules"] = m_mockRules;
    sendCommand(cmd);
}

QVariantList ProxyRulesManager::getMockRules() const
{
    return m_mockRules.toVariantList();
}

// ====================== Breakpoint Rules ======================

void ProxyRulesManager::addBreakpointRule(const QString& urlPattern, const QString& method)
{
    QJsonObject rule;
    rule["url_pattern"] = urlPattern;
    rule["method"] = method;
    m_breakpointRules.append(rule);

    QJsonObject cmd;
    cmd["type"] = QStringLiteral("update_breakpoint_rules");
    cmd["rules"] = m_breakpointRules;
    sendCommand(cmd);
}

void ProxyRulesManager::removeBreakpointRule(int index)
{
    if (index >= 0 && index < m_breakpointRules.size()) {
        m_breakpointRules.removeAt(index);
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_breakpoint_rules");
        cmd["rules"] = m_breakpointRules;
        sendCommand(cmd);
    }
}

void ProxyRulesManager::clearBreakpointRules()
{
    m_breakpointRules = QJsonArray();
    QJsonObject cmd;
    cmd["type"] = QStringLiteral("update_breakpoint_rules");
    cmd["rules"] = m_breakpointRules;
    sendCommand(cmd);
}

QVariantList ProxyRulesManager::getBreakpointRules() const
{
    return m_breakpointRules.toVariantList();
}

// ====================== Blacklist ======================

void ProxyRulesManager::addBlacklistRule(const QString& urlPattern)
{
    QJsonObject rule;
    rule["url_pattern"] = urlPattern;
    m_blacklistRules.append(rule);

    QJsonObject cmd;
    cmd["type"] = QStringLiteral("update_blacklist");
    cmd["rules"] = m_blacklistRules;
    sendCommand(cmd);
}

void ProxyRulesManager::removeBlacklistRule(int index)
{
    if (index >= 0 && index < m_blacklistRules.size()) {
        m_blacklistRules.removeAt(index);
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_blacklist");
        cmd["rules"] = m_blacklistRules;
        sendCommand(cmd);
    }
}

QVariantList ProxyRulesManager::getBlacklistRules() const
{
    return m_blacklistRules.toVariantList();
}

// ====================== Map Local ======================

void ProxyRulesManager::addMapLocalRule(const QString& urlPattern, const QString& localPath)
{
    QJsonObject rule;
    rule["url_pattern"] = urlPattern;
    rule["local_path"] = localPath;
    m_mapLocalRules.append(rule);

    QJsonObject cmd;
    cmd["type"] = QStringLiteral("update_map_local");
    cmd["rules"] = m_mapLocalRules;
    sendCommand(cmd);
}

void ProxyRulesManager::removeMapLocalRule(int index)
{
    if (index >= 0 && index < m_mapLocalRules.size()) {
        m_mapLocalRules.removeAt(index);
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_map_local");
        cmd["rules"] = m_mapLocalRules;
        sendCommand(cmd);
    }
}

QVariantList ProxyRulesManager::getMapLocalRules() const
{
    return m_mapLocalRules.toVariantList();
}

// ====================== Map Remote ======================

void ProxyRulesManager::addMapRemoteRule(const QString& srcPattern, const QString& destUrl)
{
    QJsonObject rule;
    rule["src_pattern"] = srcPattern;
    rule["dest_url"] = destUrl;
    m_mapRemoteRules.append(rule);

    QJsonObject cmd;
    cmd["type"] = QStringLiteral("update_map_remote");
    cmd["rules"] = m_mapRemoteRules;
    sendCommand(cmd);
}

void ProxyRulesManager::removeMapRemoteRule(int index)
{
    if (index >= 0 && index < m_mapRemoteRules.size()) {
        m_mapRemoteRules.removeAt(index);
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_map_remote");
        cmd["rules"] = m_mapRemoteRules;
        sendCommand(cmd);
    }
}

QVariantList ProxyRulesManager::getMapRemoteRules() const
{
    return m_mapRemoteRules.toVariantList();
}

// ====================== Throttle ======================

void ProxyRulesManager::setThrottle(bool enabled, int downloadKBps, int uploadKBps)
{
    QJsonObject cmd;
    cmd["type"] = QStringLiteral("set_throttle");
    cmd["enabled"] = enabled;
    cmd["download_kbps"] = downloadKBps;
    cmd["upload_kbps"] = uploadKBps;
    sendCommand(cmd);
}

// ====================== URL Pattern Testing ======================

QString ProxyRulesManager::testUrlPattern(const QString& pattern, const QString& testUrl)
{
    if (pattern.isEmpty())
        return tr("✗ Empty pattern");

    QRegularExpression re(pattern);
    if (!re.isValid())
        return tr("✗ Invalid regex: %1").arg(re.errorString());

    QRegularExpressionMatch match = re.match(testUrl);
    if (match.hasMatch())
        return tr("✓ Match! Captured: \"%1\"").arg(match.captured(0));
    else
        return tr("✗ No match");
}

// ====================== Send All Rules ======================

void ProxyRulesManager::sendAllRules()
{
    if (!m_mockRules.isEmpty()) {
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_mock_rules");
        cmd["rules"] = m_mockRules;
        sendCommand(cmd);
    }
    if (!m_breakpointRules.isEmpty()) {
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_breakpoint_rules");
        cmd["rules"] = m_breakpointRules;
        sendCommand(cmd);
    }
    if (!m_blacklistRules.isEmpty()) {
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_blacklist");
        cmd["rules"] = m_blacklistRules;
        sendCommand(cmd);
    }
    if (!m_mapLocalRules.isEmpty()) {
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_map_local");
        cmd["rules"] = m_mapLocalRules;
        sendCommand(cmd);
    }
    if (!m_mapRemoteRules.isEmpty()) {
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_map_remote");
        cmd["rules"] = m_mapRemoteRules;
        sendCommand(cmd);
    }
}
