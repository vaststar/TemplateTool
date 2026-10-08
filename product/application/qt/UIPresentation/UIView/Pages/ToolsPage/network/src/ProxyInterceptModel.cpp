#include "ToolsPage/network/ProxyInterceptModel.h"

ProxyInterceptModel::ProxyInterceptModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ProxyInterceptModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : count();
}

QVariant ProxyInterceptModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= count())
        return {};

    const Entry& entry = m_entries[index.row()];
    switch (role) {
    case FlowIdRole:        return entry.flowId;
    case MethodRole:        return entry.method;
    case UrlRole:           return entry.url;
    case ActionPendingRole: return entry.actionPending;
    default:               return {};
    }
}

QHash<int, QByteArray> ProxyInterceptModel::roleNames() const
{
    return {{FlowIdRole, "flowId"}, {MethodRole, "method"},
            {UrlRole, "url"}, {ActionPendingRole, "actionPending"}};
}

int ProxyInterceptModel::count() const
{
    return static_cast<int>(m_entries.size());
}

bool ProxyInterceptModel::addOrUpdate(const QString& flowId, const QJsonObject& detail)
{
    if (flowId.isEmpty() || flowId.size() > kMaxFlowIdLength)
        return false;

    const QString method = detail.value(QStringLiteral("method")).toString().left(kMaxMethodLength);
    const QString url = detail.value(QStringLiteral("url")).toString().left(kMaxUrlLength);
    const auto found = m_flowIdIndex.constFind(flowId);
    if (found != m_flowIdIndex.cend()) {
        const int row = found.value();
        Entry& entry = m_entries[row];
        entry.method = method;
        entry.url = url;
        // Duplicate notifications must not re-enable an operation awaiting acknowledgement.
        const QModelIndex changed = index(row, 0);
        emit dataChanged(changed, changed, {MethodRole, UrlRole});
        return true;
    }
    if (count() >= kMaxPendingFlows)
        return false;

    const int row = count();
    beginInsertRows({}, row, row);
    m_entries.append({flowId, method, url, false});
    m_flowIdIndex.insert(flowId, row);
    endInsertRows();
    emit countChanged();
    return true;
}

bool ProxyInterceptModel::contains(const QString& flowId) const
{
    return m_flowIdIndex.contains(flowId);
}

bool ProxyInterceptModel::isActionPending(const QString& flowId) const
{
    const auto found = m_flowIdIndex.constFind(flowId);
    return found != m_flowIdIndex.cend() && m_entries[found.value()].actionPending;
}

void ProxyInterceptModel::setActionPending(const QString& flowId, bool pending)
{
    const auto found = m_flowIdIndex.constFind(flowId);
    if (found == m_flowIdIndex.cend())
        return;

    const int row = found.value();
    if (m_entries[row].actionPending == pending)
        return;
    m_entries[row].actionPending = pending;
    const QModelIndex changed = index(row, 0);
    emit dataChanged(changed, changed, {ActionPendingRole});
}

void ProxyInterceptModel::remove(const QString& flowId)
{
    const auto found = m_flowIdIndex.constFind(flowId);
    if (found == m_flowIdIndex.cend())
        return;

    const int row = found.value();
    beginRemoveRows({}, row, row);
    m_entries.removeAt(row);
    m_flowIdIndex.remove(flowId);
    for (int i = row; i < count(); ++i)
        m_flowIdIndex.insert(m_entries[i].flowId, i);
    endRemoveRows();
    emit countChanged();
}

void ProxyInterceptModel::clear()
{
    if (m_entries.isEmpty())
        return;
    beginResetModel();
    m_entries.clear();
    m_flowIdIndex.clear();
    endResetModel();
    emit countChanged();
}

QStringList ProxyInterceptModel::flowIds() const
{
    QStringList ids;
    ids.reserve(count());
    for (const Entry& entry : m_entries)
        ids.append(entry.flowId);
    return ids;
}
