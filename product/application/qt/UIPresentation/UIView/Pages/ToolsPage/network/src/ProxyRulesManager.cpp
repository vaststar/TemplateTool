#include "ToolsPage/network/ProxyRulesManager.h"

#include <QJsonDocument>
#include <QRegularExpression>
#include <QUuid>

#include <cmath>
#include <limits>

namespace {
constexpr int kMaxRewriteRules = 128;
constexpr int kMaxHeaderOperations = 64;
constexpr qsizetype kMaxRewriteBytes = 1024 * 1024;
constexpr qsizetype kMaxPatternCharacters = 4096;
constexpr qsizetype kMaxHeaderNameCharacters = 256;
constexpr qsizetype kMaxHeaderValueBytes = 64 * 1024;
constexpr qsizetype kMaxPointerCharacters = 4096;
constexpr qsizetype kMaxConfigBytes = 4 * 1024 * 1024;
constexpr int kConfigAckTimeoutMs = 5000;

bool isValidJsonPointer(const QString& pointer)
{
    if (!pointer.isEmpty() && !pointer.startsWith(QLatin1Char('/')))
        return false;
    static const QRegularExpression invalidEscape(QStringLiteral("~(?:[^01]|$)"));
    return !invalidEscape.match(pointer).hasMatch();
}

bool containsNonFiniteNumber(const QVariant& value)
{
    switch (value.typeId()) {
    case QMetaType::Double:
    case QMetaType::Float:
        return !std::isfinite(value.toDouble());
    case QMetaType::QVariantMap: {
        const QVariantMap map = value.toMap();
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            if (containsNonFiniteNumber(it.value()))
                return true;
        }
        return false;
    }
    case QMetaType::QVariantHash: {
        const QVariantHash hash = value.toHash();
        for (auto it = hash.cbegin(); it != hash.cend(); ++it) {
            if (containsNonFiniteNumber(it.value()))
                return true;
        }
        return false;
    }
    case QMetaType::QVariantList:
        for (const QVariant& item : value.toList()) {
            if (containsNonFiniteNumber(item))
                return true;
        }
        return false;
    case QMetaType::QJsonObject:
        return containsNonFiniteNumber(value.value<QJsonObject>().toVariantMap());
    case QMetaType::QJsonArray:
        return containsNonFiniteNumber(value.value<QJsonArray>().toVariantList());
    case QMetaType::QJsonValue:
        return containsNonFiniteNumber(value.value<QJsonValue>().toVariant());
    default:
        return false;
    }
}
} // namespace

ProxyRulesManager::ProxyRulesManager(QObject* parent)
    : QObject(parent)
{
    m_configAckTimer.setSingleShot(true);
    m_configAckTimer.setInterval(kConfigAckTimeoutMs);
    connect(&m_configAckTimer, &QTimer::timeout, this, [this]() {
        if (m_addonConnected && m_configSyncState == QStringLiteral("pending"))
            setConfigSyncState(QStringLiteral("failed"),
                               tr("Configuration confirmation timed out. Retry to synchronize."));
    });

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

    synchronizeConfiguration();
}

void ProxyRulesManager::removeMockRule(int index)
{
    if (index >= 0 && index < m_mockRules.size()) {
        m_mockRules.removeAt(index);
        synchronizeConfiguration();
    }
}

void ProxyRulesManager::clearMockRules()
{
    m_mockRules = QJsonArray();
    synchronizeConfiguration();
}

QVariantList ProxyRulesManager::getMockRules() const
{
    return m_mockRules.toVariantList();
}

// ====================== Rewrite Rules ======================

bool ProxyRulesManager::validateRewriteRule(const QJsonObject& input, QJsonObject& result,
                                           QString& error) const
{
    const auto fail = [&error](const QString& message) {
        error = message;
        return false;
    };

    const QJsonValue patternValue = input.value(QStringLiteral("url_pattern"));
    if (!patternValue.isString() || patternValue.toString().trimmed().isEmpty())
        return fail(tr("Rewrite URL pattern cannot be empty."));
    const QString pattern = patternValue.toString();
    if (pattern.size() > kMaxPatternCharacters)
        return fail(tr("Rewrite URL pattern cannot exceed %1 characters.").arg(kMaxPatternCharacters));
    const QRegularExpression expression(pattern);
    if (!expression.isValid())
        return fail(tr("Invalid rewrite URL regex: %1").arg(expression.errorString()));

    static const QStringList methods = {
        QStringLiteral("ANY"), QStringLiteral("GET"), QStringLiteral("POST"),
        QStringLiteral("PUT"), QStringLiteral("PATCH"), QStringLiteral("DELETE"),
        QStringLiteral("HEAD"), QStringLiteral("OPTIONS"), QStringLiteral("CONNECT"),
        QStringLiteral("TRACE")
    };
    const QString method = input.value(QStringLiteral("method")).toString();
    if (!methods.contains(method))
        return fail(tr("Invalid rewrite HTTP method."));
    const QString stage = input.value(QStringLiteral("stage")).toString();
    if (stage != QStringLiteral("request") && stage != QStringLiteral("response"))
        return fail(tr("Rewrite stage must be request or response."));
    if (!input.value(QStringLiteral("enabled")).isBool())
        return fail(tr("Rewrite enabled must be a boolean."));

    if (!input.value(QStringLiteral("body")).isObject())
        return fail(tr("Rewrite body must be an object."));
    const QJsonObject inputBody = input.value(QStringLiteral("body")).toObject();
    const QString bodyOperation = inputBody.value(QStringLiteral("operation")).toString();
    QJsonObject body;
    body[QStringLiteral("operation")] = bodyOperation;
    if (bodyOperation == QStringLiteral("replace")) {
        if (!inputBody.value(QStringLiteral("value")).isString())
            return fail(tr("Replacement body must be text."));
        body[QStringLiteral("value")] = inputBody.value(QStringLiteral("value"));
    } else if (bodyOperation == QStringLiteral("json_set")
               || bodyOperation == QStringLiteral("json_remove")) {
        const QJsonValue pathValue = inputBody.value(QStringLiteral("path"));
        if (!pathValue.isString() || !isValidJsonPointer(pathValue.toString()))
            return fail(tr("Invalid JSON Pointer. Use an empty path or a path starting with /; escape ~ as ~0 and / as ~1."));
        const QString pointer = pathValue.toString();
        if (pointer.size() > kMaxPointerCharacters)
            return fail(tr("JSON Pointer cannot exceed %1 characters.").arg(kMaxPointerCharacters));
        if (bodyOperation == QStringLiteral("json_remove") && pointer.isEmpty())
            return fail(tr("JSON remove cannot target the root."));
        body[QStringLiteral("path")] = pointer;
        if (bodyOperation == QStringLiteral("json_set")) {
            if (!inputBody.contains(QStringLiteral("value")))
                return fail(tr("JSON set requires a value."));
            body[QStringLiteral("value")] = inputBody.value(QStringLiteral("value"));
        }
    } else if (bodyOperation == QStringLiteral("text_replace")) {
        const QJsonValue find = inputBody.value(QStringLiteral("find"));
        const QJsonValue replacement = inputBody.value(QStringLiteral("replacement"));
        if (!find.isString() || find.toString().isEmpty())
            return fail(tr("Text to find cannot be empty."));
        if (!replacement.isString())
            return fail(tr("Text replacement must be text."));
        body[QStringLiteral("find")] = find;
        body[QStringLiteral("replacement")] = replacement;
    } else if (bodyOperation != QStringLiteral("none")) {
        return fail(tr("Invalid rewrite body operation."));
    }

    if (!input.value(QStringLiteral("headers")).isArray())
        return fail(tr("Rewrite headers must be an array."));
    const QJsonArray inputHeaders = input.value(QStringLiteral("headers")).toArray();
    if (inputHeaders.size() > kMaxHeaderOperations)
        return fail(tr("A rewrite rule can contain at most %1 header operations.").arg(kMaxHeaderOperations));
    static const QRegularExpression headerNamePattern(
        QStringLiteral("\\A[!#$%&'*+.^_`|~0-9A-Za-z-]+\\z"));
    QJsonArray headers;
    for (const QJsonValue& value : inputHeaders) {
        if (!value.isObject())
            return fail(tr("Each rewrite header operation must be an object."));
        const QJsonObject inputHeader = value.toObject();
        const QString operation = inputHeader.value(QStringLiteral("operation")).toString();
        if (operation != QStringLiteral("set") && operation != QStringLiteral("add")
            && operation != QStringLiteral("remove"))
            return fail(tr("Invalid rewrite header operation."));
        const QString name = inputHeader.value(QStringLiteral("name")).toString();
        if (name.size() > kMaxHeaderNameCharacters)
            return fail(tr("Rewrite header name cannot exceed %1 characters.").arg(kMaxHeaderNameCharacters));
        if (!headerNamePattern.match(name).hasMatch())
            return fail(tr("Invalid rewrite header name: %1").arg(name));
        const QString lowerName = name.toLower();
        if (lowerName == QStringLiteral("content-length")
            || lowerName == QStringLiteral("transfer-encoding"))
            return fail(tr("Content-Length and Transfer-Encoding are managed automatically."));
        if (lowerName == QStringLiteral("content-encoding")
            && bodyOperation == QStringLiteral("none"))
            return fail(tr("Content-Encoding can only be changed together with a body operation."));
        QJsonObject header;
        header[QStringLiteral("operation")] = operation;
        header[QStringLiteral("name")] = name;
        if (operation != QStringLiteral("remove")) {
            const QJsonValue headerValue = inputHeader.value(QStringLiteral("value"));
            if (!headerValue.isString())
                return fail(tr("Rewrite header value must be text."));
            const QString text = headerValue.toString();
            if (text.toUtf8().size() > kMaxHeaderValueBytes)
                return fail(tr("Rewrite header value cannot exceed 64 KiB."));
            for (const QChar character : text) {
                if (character.unicode() < 0x20 && character != QLatin1Char('\t'))
                    return fail(tr("Rewrite header values cannot contain control characters except TAB."));
            }
            header[QStringLiteral("value")] = headerValue;
        }
        headers.append(header);
    }

    result[QStringLiteral("enabled")] = input.value(QStringLiteral("enabled"));
    result[QStringLiteral("url_pattern")] = pattern;
    result[QStringLiteral("method")] = method;
    result[QStringLiteral("stage")] = stage;
    result[QStringLiteral("headers")] = headers;
    result[QStringLiteral("body")] = body;
    return true;
}

bool ProxyRulesManager::saveRewriteRule(int index, const QVariantMap& rule)
{
    if (index < -1 || index >= m_rewriteRules.size()) {
        setRewriteError(tr("Invalid rewrite rule index."));
        return false;
    }
    if (index == -1 && m_rewriteRules.size() >= kMaxRewriteRules) {
        setRewriteError(tr("At most %1 rewrite rules can be saved.").arg(kMaxRewriteRules));
        return false;
    }
    // Check before QJson conversion, which silently turns NaN and infinity into null.
    const QVariantMap body = rule.value(QStringLiteral("body")).toMap();
    if (body.value(QStringLiteral("operation")).toString() == QStringLiteral("json_set")
        && containsNonFiniteNumber(body.value(QStringLiteral("value")))) {
        setRewriteError(tr("JSON set value cannot contain non-finite numbers."));
        return false;
    }
    QJsonObject normalized;
    QString error;
    if (!validateRewriteRule(QJsonObject::fromVariantMap(rule), normalized, error)) {
        setRewriteError(error);
        return false;
    }
    normalized[QStringLiteral("id")] = index == -1
        ? QUuid::createUuid().toString(QUuid::WithoutBraces)
        : m_rewriteRules.at(index).toObject().value(QStringLiteral("id")).toString();
    QJsonArray candidate = m_rewriteRules;
    if (index == -1)
        candidate.append(normalized);
    else
        candidate.replace(index, normalized);
    if (QJsonDocument(candidate).toJson(QJsonDocument::Compact).size() > kMaxRewriteBytes) {
        setRewriteError(tr("Rewrite rules exceed the 1 MiB configuration limit."));
        return false;
    }
    m_rewriteRules = candidate;
    setRewriteError(QString());
    publishRewriteRules();
    return true;
}

void ProxyRulesManager::removeRewriteRule(int index)
{
    if (index < 0 || index >= m_rewriteRules.size()) {
        setRewriteError(tr("Invalid rewrite rule index."));
        return;
    }
    m_rewriteRules.removeAt(index);
    setRewriteError(QString());
    publishRewriteRules();
}

void ProxyRulesManager::clearRewriteRules()
{
    m_rewriteRules = QJsonArray();
    setRewriteError(QString());
    publishRewriteRules();
}

void ProxyRulesManager::setRewriteRuleEnabled(int index, bool enabled)
{
    if (index < 0 || index >= m_rewriteRules.size()) {
        setRewriteError(tr("Invalid rewrite rule index."));
        return;
    }
    QJsonObject rule = m_rewriteRules.at(index).toObject();
    setRewriteError(QString());
    if (rule.value(QStringLiteral("enabled")).toBool() == enabled)
        return;
    rule[QStringLiteral("enabled")] = enabled;
    QJsonArray candidate = m_rewriteRules;
    candidate.replace(index, rule);
    if (QJsonDocument(candidate).toJson(QJsonDocument::Compact).size() > kMaxRewriteBytes) {
        setRewriteError(tr("Rewrite rules exceed the 1 MiB configuration limit."));
        return;
    }
    m_rewriteRules = candidate;
    publishRewriteRules();
}

void ProxyRulesManager::moveRewriteRule(int from, int to)
{
    if (from < 0 || from >= m_rewriteRules.size() || to < 0 || to >= m_rewriteRules.size()) {
        setRewriteError(tr("Invalid rewrite rule index."));
        return;
    }
    setRewriteError(QString());
    if (from == to)
        return;
    const QJsonValue rule = m_rewriteRules.at(from);
    m_rewriteRules.removeAt(from);
    m_rewriteRules.insert(to, rule);
    publishRewriteRules();
}

QVariantList ProxyRulesManager::getRewriteRules() const
{
    return m_rewriteRules.toVariantList();
}

void ProxyRulesManager::setRewriteError(const QString& error)
{
    if (m_rewriteError == error)
        return;
    m_rewriteError = error;
    emit rewriteErrorChanged();
}

void ProxyRulesManager::publishRewriteRules()
{
    emit rewriteRulesChanged();
    synchronizeConfiguration();
}

// ====================== Breakpoint Rules ======================

void ProxyRulesManager::addBreakpointRule(const QString& urlPattern, const QString& method)
{
    QJsonObject rule;
    rule["url_pattern"] = urlPattern;
    rule["method"] = method;
    m_breakpointRules.append(rule);

    synchronizeConfiguration();
}

void ProxyRulesManager::removeBreakpointRule(int index)
{
    if (index >= 0 && index < m_breakpointRules.size()) {
        m_breakpointRules.removeAt(index);
        synchronizeConfiguration();
    }
}

void ProxyRulesManager::clearBreakpointRules()
{
    m_breakpointRules = QJsonArray();
    synchronizeConfiguration();
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

    synchronizeConfiguration();
}

void ProxyRulesManager::removeBlacklistRule(int index)
{
    if (index >= 0 && index < m_blacklistRules.size()) {
        m_blacklistRules.removeAt(index);
        synchronizeConfiguration();
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

    synchronizeConfiguration();
}

void ProxyRulesManager::removeMapLocalRule(int index)
{
    if (index >= 0 && index < m_mapLocalRules.size()) {
        m_mapLocalRules.removeAt(index);
        synchronizeConfiguration();
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

    synchronizeConfiguration();
}

void ProxyRulesManager::removeMapRemoteRule(int index)
{
    if (index >= 0 && index < m_mapRemoteRules.size()) {
        m_mapRemoteRules.removeAt(index);
        synchronizeConfiguration();
    }
}

QVariantList ProxyRulesManager::getMapRemoteRules() const
{
    return m_mapRemoteRules.toVariantList();
}

// ====================== Throttle ======================

void ProxyRulesManager::setThrottle(bool enabled, int downloadKBps, int uploadKBps)
{
    const int download = qMax(0, downloadKBps);
    const int upload = qMax(0, uploadKBps);
    if (m_throttleEnabled == enabled && m_downloadKbps == download && m_uploadKbps == upload)
        return;
    m_throttleEnabled = enabled;
    m_downloadKbps = download;
    m_uploadKbps = upload;
    emit throttleChanged();
    synchronizeConfiguration();
}

void ProxyRulesManager::setBypassHosts(const QStringList& hostPatterns)
{
    m_bypassHosts.clear();
    for (const QString& pattern : hostPatterns) {
        QString trimmed = pattern.trimmed();
        if (!trimmed.isEmpty())
            m_bypassHosts.append(trimmed);
    }

    synchronizeConfiguration();
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

// ====================== Configuration Synchronization ======================

void ProxyRulesManager::setInterceptEnabled(bool enabled)
{
    if (m_interceptEnabled == enabled)
        return;
    m_interceptEnabled = enabled;
    synchronizeConfiguration();
}

void ProxyRulesManager::setAddonConnected(bool connected)
{
    if (!connected) {
        m_configAckTimer.stop();
        m_addonConnected = false;
        m_configSessionId.clear();
        setConfigSyncState(QStringLiteral("pending"));
        return;
    }
    if (m_addonConnected)
        return;
    m_addonConnected = true;
    m_configSessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    if (m_configRevision == std::numeric_limits<quint64>::max())
        m_configRevision = 0;
    synchronizeConfiguration();
}

void ProxyRulesManager::handleConfigResult(const QString& sessionId, const QString& revision,
                                         bool accepted, const QString& message)
{
    // Acknowledgements from an older edit, connection or timed-out attempt cannot
    // make the current desired configuration appear applied.
    if (!m_addonConnected || m_configSyncState != QStringLiteral("pending")
        || sessionId != m_configSessionId || revision != getConfigRevision())
        return;
    m_configAckTimer.stop();
    if (accepted) {
        setConfigSyncState(QStringLiteral("applied"));
    } else {
        const QString error = message.trimmed().left(4096);
        setConfigSyncState(QStringLiteral("failed"), error.isEmpty()
            ? tr("The proxy rejected the configuration.") : error);
    }
}

QJsonObject ProxyRulesManager::configurationSnapshot() const
{
    QJsonArray hosts;
    for (const QString& host : m_bypassHosts)
        hosts.append(host);

    const QJsonObject throttle{
        {QStringLiteral("enabled"), m_throttleEnabled},
        {QStringLiteral("download_kbps"), m_downloadKbps},
        {QStringLiteral("upload_kbps"), m_uploadKbps}
    };
    return {
        {QStringLiteral("mock_rules"), m_mockRules},
        {QStringLiteral("breakpoint_rules"), m_breakpointRules},
        {QStringLiteral("blacklist_rules"), m_blacklistRules},
        {QStringLiteral("map_local_rules"), m_mapLocalRules},
        {QStringLiteral("map_remote_rules"), m_mapRemoteRules},
        {QStringLiteral("rewrite_rules"), m_rewriteRules},
        {QStringLiteral("bypass_hosts"), hosts},
        {QStringLiteral("throttle"), throttle},
        {QStringLiteral("intercept_enabled"), m_interceptEnabled}
    };
}

void ProxyRulesManager::synchronizeConfiguration()
{
    m_configAckTimer.stop();
    if (m_configRevision == std::numeric_limits<quint64>::max()) {
        // The backend binds one session to one TCP connection. A new connection
        // can reset the version; never wrap or rotate the session while connected.
        setConfigSyncState(QStringLiteral("failed"),
                           tr("Configuration version limit reached. Reconnect the proxy to synchronize."));
        return;
    }
    ++m_configRevision;
    m_configSyncState = QStringLiteral("pending");
    m_configSyncError.clear();
    emit configSyncChanged();
    if (!m_addonConnected)
        return;

    const QJsonObject config = configurationSnapshot();
    if (QJsonDocument(config).toJson(QJsonDocument::Compact).size() > kMaxConfigBytes) {
        setConfigSyncState(QStringLiteral("failed"),
                           tr("Proxy configuration exceeds the 4 MiB limit. Remove or reduce rules before retrying."));
        return;
    }
    if (!m_sendCommandFn) {
        setConfigSyncState(QStringLiteral("failed"),
                           tr("Proxy command transport is unavailable."));
        return;
    }
    const QJsonObject command{
        {QStringLiteral("type"), QStringLiteral("apply_proxy_config")},
        {QStringLiteral("session_id"), m_configSessionId},
        {QStringLiteral("revision"), getConfigRevision()},
        {QStringLiteral("config"), config}
    };
    // Start before sending: a synchronous transport failure can disconnect and
    // stop this timer from inside the callback.
    m_configAckTimer.start();
    sendCommand(command);
}

void ProxyRulesManager::setConfigSyncState(const QString& state, const QString& error)
{
    if (m_configSyncState == state && m_configSyncError == error)
        return;
    m_configSyncState = state;
    m_configSyncError = error;
    emit configSyncChanged();
}

void ProxyRulesManager::retryConfigSync()
{
    synchronizeConfiguration();
}

void ProxyRulesManager::sendAllRules()
{
    synchronizeConfiguration();
}
