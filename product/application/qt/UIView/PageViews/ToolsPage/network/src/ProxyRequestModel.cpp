#include "PageViews/ToolsPage/network/include/ProxyRequestModel.h"

#include <QJsonDocument>
#include <QUrl>

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

void ProxyRequestModel::addOrUpdateRequest(const QJsonObject& msg)
{
    QString flowId = msg["flow_id"].toString();
    if (flowId.isEmpty())
        return;

    auto it = m_flowIdIndex.find(flowId);
    if (it != m_flowIdIndex.end()) {
        // Update existing entry
        int row = it.value();
        auto& e = m_entries[row];

        // Merge response data into the entry
        if (msg.contains("status_code"))
            e.statusCode = msg["status_code"].toInt();
        if (msg.contains("response_content_type"))
            e.contentType = msg["response_content_type"].toString();
        if (msg.contains("response_content_length"))
            e.contentLength = msg["response_content_length"].toVariant().toLongLong();
        if (msg.contains("duration"))
            e.duration = msg["duration"].toDouble();
        if (msg.contains("is_intercepted"))
            e.isIntercepted = msg["is_intercepted"].toBool();
        if (msg.contains("process_name"))
            e.processName = msg["process_name"].toString();

        // Merge full data
        for (auto jt = msg.begin(); jt != msg.end(); ++jt)
            e.fullData[jt.key()] = jt.value();

        QModelIndex idx = index(row, 0);
        emit dataChanged(idx, idx);
    } else {
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

        // Parse host/path from URL
        QUrl parsedUrl(e.url);
        e.host = parsedUrl.host();
        e.path = parsedUrl.path();

        // Also store host/path in fullData for export
        e.fullData["host"] = e.host;
        e.fullData["path"] = e.path;

        int row = static_cast<int>(m_entries.size());
        beginInsertRows(QModelIndex(), row, row);
        m_entries.append(e);
        m_flowIdIndex[flowId] = row;
        endInsertRows();
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
