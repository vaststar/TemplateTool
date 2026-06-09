#include "ToolsPage/common/ToolsTreeModel.h"

#include <functional>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace {
constexpr const char* kRootId = "";
} // namespace

ToolsTreeModel::ToolsTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    resetMirror();
}

ToolsTreeModel::~ToolsTreeModel() = default;

// ---------------- Public mutation API ----------------

void ToolsTreeModel::resetFromTree(const TreePtr& tree)
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

void ToolsTreeModel::insertNodes(const std::vector<NodeData>& datas)
{
    // Group fresh nodes by parentId so we can fire one beginInsertRows per
    // parent. Unknown parents and duplicate ids are dropped.
    std::unordered_map<std::string, std::vector<NodeData>> bucketByParent;
    for (const auto& d : datas)
    {
        if (d.nodeId.empty() || m_nodes.count(d.nodeId))
        {
            continue;
        }
        if (!findNode(d.parentId))
        {
            UIVIEW_LOG_WARN("insertNodes: unknown parent "
                            << d.parentId << " for node " << d.nodeId);
            continue;
        }
        bucketByParent[d.parentId].push_back(d);
    }

    for (auto& [parentId, fresh] : bucketByParent)
    {
        Node* parentNode = findNode(parentId);
        if (!parentNode || fresh.empty())
        {
            continue;
        }
        const int start = static_cast<int>(parentNode->childIds.size());
        const int end   = start + static_cast<int>(fresh.size()) - 1;
        QModelIndex parentIdx = indexFor(parentNode);
        beginInsertRows(parentIdx, start, end);
        for (const auto& d : fresh)
        {
            auto node = std::make_unique<Node>();
            node->data = d;
            node->parentId = parentId;
            node->rowInParent = static_cast<int>(parentNode->childIds.size());
            parentNode->childIds.push_back(d.nodeId);
            m_nodes.emplace(d.nodeId, std::move(node));
        }
        endInsertRows();
    }
}

void ToolsTreeModel::updateNodes(const std::vector<NodeData>& datas)
{
    for (const auto& d : datas)
    {
        Node* node = findNode(d.nodeId);
        if (!node || node == getRoot())
        {
            continue;
        }
        // updateNodes is for in-place property refresh; parent/id are stable.
        // If parentId changed, ignore it here — a real reparent would be a
        // remove + insert sequence.
        const std::string keepParent = node->parentId;
        node->data = d;
        node->data.parentId = keepParent;

        QModelIndex idx = indexFor(node);
        if (idx.isValid())
        {
            emit dataChanged(idx, idx);
        }
    }
}

void ToolsTreeModel::removeNodes(const std::vector<std::string>& ids)
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

        // Strict tree on UI side; if a category gets removed we drop the
        // whole subtree rather than re-parenting orphans (no drag/drop).
        std::vector<std::string> subtree;
        std::function<void(Node*)> gather = [&](Node* n) {
            for (const auto& cid : n->childIds)
            {
                if (Node* c = findNode(cid))
                {
                    gather(c);
                    subtree.push_back(cid);
                }
            }
        };
        gather(node);

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
        for (const auto& descendantId : subtree)
        {
            m_nodes.erase(descendantId);
        }
        m_nodes.erase(id);
        endRemoveRows();
    }
}

QModelIndex ToolsTreeModel::indexOfId(const QString& id) const
{
    Node* n = findNode(id.toStdString());
    if (!n || n == getRoot()) return {};
    return indexFor(n);
}

// ---------------- QAbstractItemModel ----------------

QVariant ToolsTreeModel::data(const QModelIndex& index, int role) const
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
    case TitleRole:
        return QString::fromStdString(d.title);
    case NodeIdRole:
        return QString::fromStdString(d.nodeId);
    case IconRole:
        return QString::fromStdString(d.icon);
    case PanelTypeRole:
        return static_cast<int>(d.panelType);
    default:
        return {};
    }
}

Qt::ItemFlags ToolsTreeModel::flags(const QModelIndex& index) const
{
    return index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable)
                           : Qt::NoItemFlags;
}

QModelIndex ToolsTreeModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex ToolsTreeModel::parent(const QModelIndex& index) const
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

int ToolsTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
    {
        return 0;
    }
    Node* parentNode = nodeFromIndex(parent);
    return parentNode ? static_cast<int>(parentNode->childIds.size()) : 0;
}

int ToolsTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QHash<int, QByteArray> ToolsTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NodeIdRole]    = "nodeId";
    roles[TitleRole]     = "title";
    roles[IconRole]      = "icon";
    roles[PanelTypeRole] = "panelType";
    return roles;
}

// ---------------- Internal helpers ----------------

ToolsTreeModel::Node* ToolsTreeModel::getRoot() const
{
    auto it = m_nodes.find(std::string(kRootId));
    return (it != m_nodes.end()) ? it->second.get() : nullptr;
}

ToolsTreeModel::Node* ToolsTreeModel::findNode(const std::string& id) const
{
    auto it = m_nodes.find(id);
    return (it != m_nodes.end()) ? it->second.get() : nullptr;
}

ToolsTreeModel::Node* ToolsTreeModel::nodeFromIndex(const QModelIndex& idx) const
{
    if (!idx.isValid())
    {
        return getRoot();
    }
    return static_cast<Node*>(idx.internalPointer());
}

QModelIndex ToolsTreeModel::indexFor(Node* node) const
{
    if (!node || node == getRoot())
    {
        return {};
    }
    return createIndex(node->rowInParent, 0, static_cast<void*>(node));
}

void ToolsTreeModel::resetMirror()
{
    m_nodes.clear();
    auto root = std::make_unique<Node>();
    root->parentId = kRootId;
    root->rowInParent = -1;
    m_nodes.emplace(std::string(kRootId), std::move(root));
}

void ToolsTreeModel::walkPopulate(const TreeNodePtr& src,
                                  const std::string& parentId)
{
    if (!src)
    {
        return;
    }
    auto data = src->getNodeData();
    if (data.nodeId.empty() || m_nodes.count(data.nodeId))
    {
        return;
    }
    Node* parentNode = findNode(parentId);
    if (!parentNode)
    {
        return;
    }
    // Make sure the mirror's parentId field always matches where we hung the
    // node, even if the source forgot to set it.
    data.parentId = parentId;

    auto node = std::make_unique<Node>();
    node->data = data;
    node->parentId = parentId;
    node->rowInParent = static_cast<int>(parentNode->childIds.size());
    parentNode->childIds.push_back(data.nodeId);
    m_nodes.emplace(data.nodeId, std::move(node));

    const std::size_t n = src->getChildCount();
    for (std::size_t i = 0; i < n; ++i)
    {
        walkPopulate(src->getChild(i), data.nodeId);
    }
}
