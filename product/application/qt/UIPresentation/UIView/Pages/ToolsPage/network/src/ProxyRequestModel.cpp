#include "ToolsPage/network/ProxyRequestModel.h"

#include <QJsonDocument>
#include <QSet>
#include <QUrl>

#include <utility>

// ======================== ProxyRequestModel ========================

ProxyRequestModel::ProxyRequestModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ProxyRequestModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_entries.size());
}

QVariant ProxyRequestModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return QVariant();

    const auto& e = m_entries[index.row()];

    switch (role) {
    case FlowIdRole:        return e.flowId;
    case MethodRole:        return e.method;
    case UrlRole:           return e.url;
    case HostRole:          return e.host;
    case PathRole:          return e.path;
    case StatusCodeRole:    return e.statusCode;
    case ContentTypeRole:   return e.contentType;
    case ContentLengthRole: return e.contentLength;
    case DurationRole:      return e.duration;
    case TimestampRole:     return e.timestamp;
    case ProcessNameRole:   return e.processName;
    case IsHttpsRole:       return e.isHttps;
    case IsWebSocketRole:   return e.isWebSocket;
    case IsInterceptedRole: return e.isIntercepted;
    case FullDataRole: {
        QJsonDocument doc(e.fullData);
        return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ProxyRequestModel::roleNames() const
{
    return {
        { FlowIdRole,        "flowId" },
        { MethodRole,        "method" },
        { UrlRole,           "url" },
        { HostRole,          "host" },
        { PathRole,          "path" },
        { StatusCodeRole,    "statusCode" },
        { ContentTypeRole,   "contentType" },
        { ContentLengthRole, "contentLength" },
        { DurationRole,      "duration" },
        { TimestampRole,     "timestamp" },
        { ProcessNameRole,   "processName" },
        { IsHttpsRole,       "isHttps" },
        { IsWebSocketRole,   "isWebSocket" },
        { IsInterceptedRole, "isIntercepted" },
        { FullDataRole,      "fullData" },
    };
}

qsizetype ProxyRequestModel::estimateBytes(const RequestEntry& entry)
{
    // The compact JSON size plus duplicate strings is a retention estimate,
    // not an exact measurement of Qt container or allocator overhead.
    const auto textBytes = [](const QString& text) -> qsizetype {
        return text.size() * qsizetype(sizeof(QChar));
    };

    return QJsonDocument(entry.fullData).toJson(QJsonDocument::Compact).size()
        + textBytes(entry.flowId)
        + textBytes(entry.method)
        + textBytes(entry.url)
        + textBytes(entry.host)
        + textBytes(entry.path)
        + textBytes(entry.contentType)
        + textBytes(entry.timestamp)
        + textBytes(entry.processName);
}

bool ProxyRequestModel::fitEntry(RequestEntry& entry)
{
    qsizetype bytes = estimateBytes(entry);
    if (bytes > kMaxEntryBytes) {
        entry.fullData.remove(QStringLiteral("request_body"));
        entry.fullData.remove(QStringLiteral("response_body"));
        entry.fullData.remove(QStringLiteral("ws_content"));
        entry.fullData.insert(QStringLiteral("capture_truncated"), true);
        bytes = estimateBytes(entry);
    }
    if (bytes > kMaxEntryBytes) {
        if (!entry.fullData.value(QStringLiteral("request_headers")).toObject().isEmpty())
            entry.fullData.insert(QStringLiteral("request_headers_truncated"), true);
        entry.fullData.remove(QStringLiteral("request_headers"));
        entry.fullData.remove(QStringLiteral("response_headers"));
        bytes = estimateBytes(entry);
    }
    if (bytes > kMaxEntryBytes)
        return false;

    entry.retainedBytes = bytes;
    return true;
}

void ProxyRequestModel::trimToLimits()
{
    qsizetype remainingBytes = m_retainedBytes;
    int removeCount = 0;
    const int currentCount = static_cast<int>(m_entries.size());
    while (removeCount < currentCount
           && (currentCount - removeCount > kMaxRows
               || remainingBytes > kMaxRetainedBytes)) {
        remainingBytes -= m_entries[removeCount].retainedBytes;
        ++removeCount;
    }
    if (removeCount == 0)
        return;

    beginRemoveRows(QModelIndex(), 0, removeCount - 1);
    m_entries.erase(m_entries.begin(), m_entries.begin() + removeCount);
    m_flowIdIndex.clear();
    for (int row = 0; row < m_entries.size(); ++row)
        m_flowIdIndex.insert(m_entries[row].flowId, row);
    m_retainedBytes = remainingBytes;
    endRemoveRows();
    emit oldestRowsEvicted(removeCount);
}

void ProxyRequestModel::addOrUpdateRequest(const QJsonObject& msg)
{
    const QString flowId = msg["flow_id"].toString();
    if (flowId.isEmpty())
        return;

    const auto it = m_flowIdIndex.constFind(flowId);
    if (it != m_flowIdIndex.cend()) {
        // Build an update off-model so an oversized response cannot pollute the
        // retained row or its byte accounting.
        const int row = it.value();
        RequestEntry candidate = m_entries[row];

        // Merge response data into the entry
        if (msg.contains("status_code"))
            candidate.statusCode = msg["status_code"].toInt();
        if (msg.contains("response_content_type"))
            candidate.contentType = msg["response_content_type"].toString();
        if (msg.contains("response_content_length"))
            candidate.contentLength = msg["response_content_length"].toVariant().toLongLong();
        if (msg.contains("duration"))
            candidate.duration = msg["duration"].toDouble();
        if (msg.contains("is_intercepted"))
            candidate.isIntercepted = msg["is_intercepted"].toBool();
        if (msg.contains("process_name"))
            candidate.processName = msg["process_name"].toString();

        // Merge full data
        for (auto jt = msg.begin(); jt != msg.end(); ++jt) {
            // This marker belongs to the retained model state and is monotonic.
            if (jt.key() != QStringLiteral("request_headers_truncated"))
                candidate.fullData[jt.key()] = jt.value();
        }

        if (!fitEntry(candidate))
            return;

        m_retainedBytes += candidate.retainedBytes - m_entries[row].retainedBytes;
        m_entries[row] = std::move(candidate);
        trimToLimits();
        const auto updated = m_flowIdIndex.constFind(flowId);
        if (updated != m_flowIdIndex.cend()) {
            const QModelIndex idx = index(updated.value(), 0);
            emit dataChanged(idx, idx);
        }
    } else {
        // A response or controller patch arriving after eviction/clear must
        // not create a partial request row.
        if (msg["type"].toString() != QStringLiteral("request"))
            return;

        // New entry
        RequestEntry e;
        e.flowId       = flowId;
        e.method       = msg["method"].toString();
        e.url          = msg["url"].toString();
        e.timestamp    = msg["timestamp"].toString();
        e.isHttps      = msg["is_https"].toBool();
        e.isWebSocket  = msg["is_websocket"].toBool();
        e.isIntercepted = msg["is_intercepted"].toBool();
        e.processName  = msg["process_name"].toString();
        e.statusCode   = msg["status_code"].toInt();
        e.contentType  = msg["response_content_type"].toString();
        e.contentLength = msg["response_content_length"].toVariant().toLongLong();
        e.duration     = msg["duration"].toDouble();
        e.fullData     = msg;
        e.fullData.remove(QStringLiteral("request_headers_truncated"));

        // Parse host/path from URL
        QUrl parsedUrl(e.url);
        e.host = parsedUrl.host();
        e.path = parsedUrl.path();

        // Also store host/path in fullData for export
        e.fullData["host"] = e.host;
        e.fullData["path"] = e.path;

        if (!fitEntry(e))
            return;

        int row = static_cast<int>(m_entries.size());
        beginInsertRows(QModelIndex(), row, row);
        m_entries.append(e);
        m_flowIdIndex[flowId] = row;
        m_retainedBytes += e.retainedBytes;
        endInsertRows();
        trimToLimits();
    }
}

QJsonObject ProxyRequestModel::getRequestAt(int row) const
{
    if (row >= 0 && row < m_entries.size())
        return m_entries[row].fullData;
    return QJsonObject();
}

void ProxyRequestModel::clear()
{
    beginResetModel();
    m_entries.clear();
    m_flowIdIndex.clear();
    m_retainedBytes = 0;
    endResetModel();
}

QStringList ProxyRequestModel::uniqueProcessNames() const
{
    QSet<QString> names;
    for (const auto& e : m_entries) {
        if (!e.processName.isEmpty())
            names.insert(e.processName);
    }
    QStringList sorted = names.values();
    sorted.sort(Qt::CaseInsensitive);
    return sorted;
}

// ======================== ProxyFilterModel ========================

ProxyFilterModel::ProxyFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

QString ProxyFilterModel::filterUrl() const { return m_filterUrl; }
QString ProxyFilterModel::filterMethod() const { return m_filterMethod; }
QString ProxyFilterModel::filterStatus() const { return m_filterStatus; }
QString ProxyFilterModel::filterContentType() const { return m_filterContentType; }
QString ProxyFilterModel::filterProcess() const { return m_filterProcess; }

void ProxyFilterModel::setFilterUrl(const QString& v)
{
    if (m_filterUrl != v) {
        m_filterUrl = v;
        invalidateFilter();
        emit filterUrlChanged();
    }
}

void ProxyFilterModel::setFilterMethod(const QString& v)
{
    if (m_filterMethod != v) {
        m_filterMethod = v;
        invalidateFilter();
        emit filterMethodChanged();
    }
}

void ProxyFilterModel::setFilterStatus(const QString& v)
{
    if (m_filterStatus != v) {
        m_filterStatus = v;
        invalidateFilter();
        emit filterStatusChanged();
    }
}

void ProxyFilterModel::setFilterContentType(const QString& v)
{
    if (m_filterContentType != v) {
        m_filterContentType = v;
        invalidateFilter();
        emit filterContentTypeChanged();
    }
}

void ProxyFilterModel::setFilterProcess(const QString& v)
{
    if (m_filterProcess != v) {
        m_filterProcess = v;
        invalidateFilter();
        emit filterProcessChanged();
    }
}

bool ProxyFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);

    // URL filter
    if (!m_filterUrl.isEmpty()) {
        QString url = idx.data(ProxyRequestModel::UrlRole).toString();
        if (!url.contains(m_filterUrl, Qt::CaseInsensitive))
            return false;
    }

    // Method filter (supports multi-select: comma-separated like "GET,POST")
    if (!m_filterMethod.isEmpty() && m_filterMethod != "ALL") {
        QString method = idx.data(ProxyRequestModel::MethodRole).toString();
        QStringList selected = m_filterMethod.split(',', Qt::SkipEmptyParts);
        if (!selected.contains(method))
            return false;
    }

    // Status code filter (supports multi-select: comma-separated like "2xx,4xx")
    if (!m_filterStatus.isEmpty() && m_filterStatus != "ALL") {
        int code = idx.data(ProxyRequestModel::StatusCodeRole).toInt();
        QStringList selectedStatuses = m_filterStatus.split(',', Qt::SkipEmptyParts);
        bool matched = false;
        for (const QString& s : selectedStatuses) {
            if (s.endsWith("xx")) {
                int century = s.left(1).toInt() * 100;
                if (code >= century && code < century + 100) {
                    matched = true;
                    break;
                }
            } else {
                if (code == s.toInt()) {
                    matched = true;
                    break;
                }
            }
        }
        if (!matched)
            return false;
    }

    // Content type filter
    if (!m_filterContentType.isEmpty() && m_filterContentType != "ALL") {
        QString ct = idx.data(ProxyRequestModel::ContentTypeRole).toString();
        if (!ct.contains(m_filterContentType, Qt::CaseInsensitive))
            return false;
    }

    // Process name filter (supports multi-select: comma-separated like "chrome,curl")
    if (!m_filterProcess.isEmpty() && m_filterProcess != "ALL") {
        QString proc = idx.data(ProxyRequestModel::ProcessNameRole).toString();
        QStringList selectedProcs = m_filterProcess.split(',', Qt::SkipEmptyParts);
        if (!selectedProcs.contains(proc))
            return false;
    }

    return true;
}
