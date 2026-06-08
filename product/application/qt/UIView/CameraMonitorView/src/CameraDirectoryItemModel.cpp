#include "CameraMonitorView/CameraDirectoryItemModel.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace {
constexpr const char* kRootId = "";
} // namespace

CameraDirectoryItemModel::CameraDirectoryItemModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    resetMirror();
}

CameraDirectoryItemModel::~CameraDirectoryItemModel() = default;

// ---------------- Public mutation API ----------------

void CameraDirectoryItemModel::resetFromTree(const TreePtr& tree)
{
    beginResetModel();
    resetMirror();
    if (tree)
    {
        if (auto srcRoot = tree->getRoot())
        {
            const std::size_t n = srcRoot->getChildCount();
            for (std::size_t i = 0; i < n; ++i)
            {
                walkPopulate(srcRoot->getChild(i), kRootId);
            }
        }
    }
    endResetModel();
}

void CameraDirectoryItemModel::insertNodes(const std::vector<NodeData>& datas)
{
    Node* root = getRoot();
    if (!root)
    {
        return;
    }
    std::vector<NodeData> fresh;
    fresh.reserve(datas.size());
    for (const auto& d : datas)
    {
        if (d.id.empty() || m_nodes.count(d.id))
        {
            continue;
        }
        fresh.push_back(d);
    }
    if (fresh.empty())
    {
        return;
    }
    const int start = static_cast<int>(root->childIds.size());
    const int end   = start + static_cast<int>(fresh.size()) - 1;
    beginInsertRows(QModelIndex(), start, end);
    for (const auto& d : fresh)
    {
        auto node = std::make_unique<Node>();
        node->data = d;
        node->parentId = kRootId;
        node->rowInParent = static_cast<int>(root->childIds.size());
        root->childIds.push_back(d.id);
        m_nodes.emplace(d.id, std::move(node));
    }
    endInsertRows();
}

void CameraDirectoryItemModel::updateNodes(const std::vector<NodeData>& datas)
{
    for (const auto& d : datas)
    {
        Node* node = findNode(d.id);
        if (!node || node == getRoot())
        {
            continue;
        }
        // Preserve id / type identity; only refresh display fields.
        node->data.displayName = d.displayName;
        node->data.status      = d.status;
        QModelIndex idx = indexFor(node);
        if (idx.isValid())
        {
            emit dataChanged(idx, idx);
        }
    }
}

void CameraDirectoryItemModel::removeNodes(const std::vector<std::string>& ids)
{
    for (const auto& id : ids)
    {
        if (id.empty())
        {
            continue;
        }
        Node* node = findNode(id);
        if (!node || node == getRoot())
        {
            continue;
        }

        // Re-parent children to root first; moveNodeToParent mutates childIds.
        std::vector<std::string> orphans = node->childIds;
        for (const auto& cid : orphans)
        {
            moveNodeToParent(cid, kRootId);
        }

        Node* parentNode = findNode(node->parentId);
        if (!parentNode)
        {
            m_nodes.erase(id);
            continue;
        }
        const int row = node->rowInParent;
        QModelIndex parentIdx = indexFor(parentNode);
        beginRemoveRows(parentIdx, row, row);
        parentNode->childIds.erase(parentNode->childIds.begin() + row);
        for (int i = row; i < static_cast<int>(parentNode->childIds.size()); ++i)
        {
            if (Node* sib = findNode(parentNode->childIds[i]))
            {
                sib->rowInParent = i;
            }
        }
        m_nodes.erase(id);
        endRemoveRows();
    }
}

void CameraDirectoryItemModel::setParents(
    const std::vector<std::pair<std::string, std::string>>& pairs)
{
    for (const auto& [parentId, childId] : pairs)
    {
        moveNodeToParent(childId, parentId);
    }
}

void CameraDirectoryItemModel::clearParents(const std::vector<std::string>& childIds)
{
    for (const auto& cid : childIds)
    {
        moveNodeToParent(cid, kRootId);
    }
}

QModelIndex CameraDirectoryItemModel::indexOfId(const QString& id) const
{
    Node* n = findNode(id.toStdString());
    if (!n || n == getRoot()) return {};
    return indexFor(n);
}

// ---------------- QAbstractItemModel ----------------

QVariant CameraDirectoryItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    Node* node = nodeFromIndex(index);
    if (!node || node == getRoot())
    {
        return {};
    }
    const auto& d = node->data;
    switch (role)
    {
    case Qt::DisplayRole:
    case DisplayNameRole: return QString::fromStdString(d.displayName);
    case IdRole:          return QString::fromStdString(d.id);
    case NodeTypeRole:
        return (d.type == commonHead::viewModels::model::CameraDirectoryNodeType::Group) ? 0 : 1;
    case StatusRole:      return static_cast<int>(d.status);
    default: return {};
    }
}

Qt::ItemFlags CameraDirectoryItemModel::flags(const QModelIndex& index) const
{
    return index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable)
                           : Qt::NoItemFlags;
}

QVariant CameraDirectoryItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
    {
        return QStringLiteral("Camera");
    }
    return {};
}

QModelIndex CameraDirectoryItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0 || column >= columnCount(parent))
    {
        return {};
    }
    Node* parentNode = nodeFromIndex(parent);
    if (!parentNode || row >= static_cast<int>(parentNode->childIds.size()))
    {
        return {};
    }
    Node* child = findNode(parentNode->childIds[row]);
    if (!child)
    {
        return {};
    }
    return createIndex(row, column, static_cast<void*>(child));
}

QModelIndex CameraDirectoryItemModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return {};
    }
    Node* child = static_cast<Node*>(index.internalPointer());
    if (!child || child->parentId == kRootId)
    {
        return {};
    }
    Node* parentNode = findNode(child->parentId);
    if (!parentNode || parentNode == getRoot())
    {
        return {};
    }
    return createIndex(parentNode->rowInParent, 0, static_cast<void*>(parentNode));
}

int CameraDirectoryItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
    {
        return 0;
    }
    Node* parentNode = nodeFromIndex(parent);
    return parentNode ? static_cast<int>(parentNode->childIds.size()) : 0;
}

int CameraDirectoryItemModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QHash<int, QByteArray> CameraDirectoryItemModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole]          = "id";
    roles[DisplayNameRole] = "displayName";
    roles[NodeTypeRole]    = "nodeType";
    roles[StatusRole]      = "status";
    return roles;
}

// ---------------- Internal helpers ----------------

CameraDirectoryItemModel::Node* CameraDirectoryItemModel::getRoot() const
{
    auto it = m_nodes.find(std::string(kRootId));
    return (it != m_nodes.end()) ? it->second.get() : nullptr;
}

CameraDirectoryItemModel::Node* CameraDirectoryItemModel::findNode(const std::string& id) const
{
    auto it = m_nodes.find(id);
    return (it != m_nodes.end()) ? it->second.get() : nullptr;
}

CameraDirectoryItemModel::Node* CameraDirectoryItemModel::nodeFromIndex(const QModelIndex& idx) const
{
    if (!idx.isValid())
    {
        return getRoot();
    }
    return static_cast<Node*>(idx.internalPointer());
}

QModelIndex CameraDirectoryItemModel::indexFor(Node* node) const
{
    if (!node || node == getRoot())
    {
        return {};
    }
    return createIndex(node->rowInParent, 0, static_cast<void*>(node));
}

void CameraDirectoryItemModel::resetMirror()
{
    m_nodes.clear();
    auto root = std::make_unique<Node>();
    root->parentId = kRootId;
    root->rowInParent = -1;
    m_nodes.emplace(std::string(kRootId), std::move(root));
}

void CameraDirectoryItemModel::walkPopulate(const TreeNodePtr& src,
                                            const std::string& parentId)
{
    if (!src)
    {
        return;
    }
    auto data = src->getNodeData();
    if (data.id.empty() || m_nodes.count(data.id))
    {
        return;
    }
    Node* parentNode = findNode(parentId);
    if (!parentNode)
    {
        return;
    }
    auto node = std::make_unique<Node>();
    node->data = data;
    node->parentId = parentId;
    node->rowInParent = static_cast<int>(parentNode->childIds.size());
    parentNode->childIds.push_back(data.id);
    m_nodes.emplace(data.id, std::move(node));

    const std::size_t n = src->getChildCount();
    for (std::size_t i = 0; i < n; ++i)
    {
        walkPopulate(src->getChild(i), data.id);
    }
}

void CameraDirectoryItemModel::moveNodeToParent(const std::string& childId,
                                                const std::string& newParentId)
{
    if (childId.empty() || childId == newParentId)
    {
        return;
    }
    Node* child = findNode(childId);
    if (!child || child == getRoot())
    {
        return;
    }
    Node* newParent = findNode(newParentId);
    if (!newParent)
    {
        return;
    }
    if (child->parentId == newParentId)
    {
        return;
    }
    Node* oldParent = findNode(child->parentId);
    if (!oldParent)
    {
        return;
    }

    const int srcRow  = child->rowInParent;
    const int destRow = static_cast<int>(newParent->childIds.size());
    QModelIndex srcIdx  = indexFor(oldParent);
    QModelIndex destIdx = indexFor(newParent);

    if (!beginMoveRows(srcIdx, srcRow, srcRow, destIdx, destRow))
    {
        UIVIEW_LOG_WARN("beginMoveRows refused: " << childId << " -> " << newParentId);
        return;
    }
    oldParent->childIds.erase(oldParent->childIds.begin() + srcRow);
    for (int i = srcRow; i < static_cast<int>(oldParent->childIds.size()); ++i)
    {
        if (Node* sib = findNode(oldParent->childIds[i]))
        {
            sib->rowInParent = i;
        }
    }
    child->parentId = newParentId;
    child->rowInParent = destRow;
    newParent->childIds.push_back(childId);
    endMoveRows();
}
