#include "ToolsPage/network/ProxyRulesManager.h"

#include <QRegularExpression>

ProxyRulesManager::ProxyRulesManager(QObject* parent)
    : QObject(parent)
{
    // Default passthrough domains for AI tools that commonly use TLS pinning.
    m_bypassHosts = {
        // OpenAI / ChatGPT / Codex
        QStringLiteral("(^|\\.)openai\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)chatgpt\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)oaistatic\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)oaiusercontent\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)openaiapi-site\\.azureedge\\.net(:\\d+)?$"),
        QStringLiteral("(^|\\.)codex\\.openai\\.com(:\\d+)?$"),
        // GitHub Copilot
        QStringLiteral("(^|\\.)githubcopilot\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)copilot\\.microsoft\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)copilot-proxy\\.githubusercontent\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)individual\\.githubcopilot\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)business\\.githubcopilot\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)enterprise\\.githubcopilot\\.com(:\\d+)?$"),
        // Anthropic / Claude
        QStringLiteral("(^|\\.)anthropic\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)claude\\.ai(:\\d+)?$"),
        // Google Gemini / Bard
        QStringLiteral("(^|\\.)gemini\\.google\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)bard\\.google\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)generativelanguage\\.googleapis\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)aistudio\\.google\\.com(:\\d+)?$"),
        // DeepSeek
        QStringLiteral("(^|\\.)deepseek\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)deepseek\\.ai(:\\d+)?$"),
        // Mistral
        QStringLiteral("(^|\\.)mistral\\.ai(:\\d+)?$"),
        // xAI / Grok
        QStringLiteral("(^|\\.)x\\.ai(:\\d+)?$"),
        QStringLiteral("(^|\\.)grok\\.com(:\\d+)?$"),
        // Perplexity
        QStringLiteral("(^|\\.)perplexity\\.ai(:\\d+)?$"),
        // Cursor
        QStringLiteral("(^|\\.)cursor\\.sh(:\\d+)?$"),
        QStringLiteral("(^|\\.)cursor\\.com(:\\d+)?$"),
        // Cody / Sourcegraph
        QStringLiteral("(^|\\.)sourcegraph\\.com(:\\d+)?$"),
        // Codeium / Windsurf
        QStringLiteral("(^|\\.)codeium\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)windsurf\\.ai(:\\d+)?$"),
        // Tabnine
        QStringLiteral("(^|\\.)tabnine\\.com(:\\d+)?$"),
        // Hugging Face
        QStringLiteral("(^|\\.)huggingface\\.co(:\\d+)?$"),
        // ByteDance Doubao / Volcengine
        QStringLiteral("(^|\\.)doubao\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)volces\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)volcengineapi\\.com(:\\d+)?$"),
        // Moonshot / Kimi
        QStringLiteral("(^|\\.)moonshot\\.cn(:\\d+)?$"),
        QStringLiteral("(^|\\.)moonshot\\.ai(:\\d+)?$"),
        QStringLiteral("(^|\\.)kimi\\.com(:\\d+)?$"),
        // Zhipu / GLM
        QStringLiteral("(^|\\.)bigmodel\\.cn(:\\d+)?$"),
        QStringLiteral("(^|\\.)zhipuai\\.cn(:\\d+)?$"),
        // Alibaba Qwen / Tongyi
        QStringLiteral("(^|\\.)tongyi\\.aliyun\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)dashscope\\.aliyuncs\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)qwen\\.ai(:\\d+)?$"),
        // Baidu Wenxin / ERNIE
        QStringLiteral("(^|\\.)wenxin\\.baidu\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)yiyan\\.baidu\\.com(:\\d+)?$"),
        // Tencent Hunyuan
        QStringLiteral("(^|\\.)hunyuan\\.tencent\\.com(:\\d+)?$"),
        // 01.AI / Yi
        QStringLiteral("(^|\\.)01\\.ai(:\\d+)?$"),
        QStringLiteral("(^|\\.)lingyiwanwu\\.com(:\\d+)?$"),
        // MiniMax
        QStringLiteral("(^|\\.)minimax\\.chat(:\\d+)?$"),
        QStringLiteral("(^|\\.)minimaxi\\.com(:\\d+)?$"),
        // Baichuan
        QStringLiteral("(^|\\.)baichuan-ai\\.com(:\\d+)?$"),
        // SenseTime
        QStringLiteral("(^|\\.)sensetime\\.com(:\\d+)?$"),
        QStringLiteral("(^|\\.)sensenova\\.cn(:\\d+)?$"),
        // Stepfun
        QStringLiteral("(^|\\.)stepfun\\.com(:\\d+)?$")
    };
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

void ProxyRulesManager::setBypassHosts(const QStringList& hostPatterns)
{
    m_bypassHosts.clear();
    for (const QString& pattern : hostPatterns) {
        QString trimmed = pattern.trimmed();
        if (!trimmed.isEmpty())
            m_bypassHosts.append(trimmed);
    }

    QJsonArray hosts;
    for (const QString& host : m_bypassHosts)
        hosts.append(host);

    QJsonObject cmd;
    cmd["type"] = QStringLiteral("update_bypass_hosts");
    cmd["hosts"] = hosts;
    sendCommand(cmd);
}

QStringList ProxyRulesManager::getBypassHosts() const
{
    return m_bypassHosts;
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

    if (!m_bypassHosts.isEmpty()) {
        QJsonArray hosts;
        for (const QString& host : m_bypassHosts)
            hosts.append(host);
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("update_bypass_hosts");
        cmd["hosts"] = hosts;
        sendCommand(cmd);
    }
}
