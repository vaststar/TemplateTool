#include "ToolsPage/textprocess/JsonTreeItemModel.h"

#include <commonHead/viewModels/JsonTreeViewModel/IJsonTreeModel.h>

JsonTreeItemModel::JsonTreeItemModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

JsonTreeItemModel::~JsonTreeItemModel() = default;

void JsonTreeItemModel::setTree(const std::shared_ptr<commonHead::viewModels::model::IJsonTree>& tree)
{
    if (m_tree == tree)
    {
        return;
    }
    beginResetModel();
    m_tree = tree;
    endResetModel();
}

void JsonTreeItemModel::clearTree()
{
    beginResetModel();
    m_tree.reset();
    endResetModel();
}

QVariant JsonTreeItemModel::data(const QModelIndex& index, int role) const
{
    if (!m_tree || !index.isValid())
    {
        return {};
    }

    auto* node = nodeFromIndex(index);
    if (!node)
    {
        return {};
    }

    auto nodeData = node->getNodeData();

    switch (role)
    {
    case KeyRole:
        return QString::fromStdString(nodeData.key);
    case ValueRole:
        return QString::fromStdString(nodeData.displayValue);
    case NodeTypeRole:
        return static_cast<int>(nodeData.type);
    case ChildCountRole:
        return static_cast<int>(nodeData.childCount);
    case ShowCommaRole:
    {
        if (nodeData.type == commonHead::viewModels::model::JsonNodeType::ClosingBracket)
        {
            auto parentWeak = node->getParent();
            auto parent = parentWeak.lock();
            if (!parent)
                return false;

            auto grandparentWeak = parent->getParent();
            auto grandparent = grandparentWeak.lock();
            if (!grandparent)
                return false;

            auto gpData = grandparent->getNodeData();
            if (gpData.type != commonHead::viewModels::model::JsonNodeType::Object && gpData.type != commonHead::viewModels::model::JsonNodeType::Array)
                return false;

            std::size_t gpChildCount = grandparent->getChildCount();
            if (gpChildCount < 2)
                return false;

            for (std::size_t i = 0; i < gpChildCount; ++i)
            {
                if (grandparent->getChild(i).get() == parent.get())
                    return i < gpChildCount - 2;
            }
            return false;
        }

        auto parentWeak = node->getParent();
        auto parent = parentWeak.lock();
        if (!parent)
            return false;

        auto parentData = parent->getNodeData();
        if (parentData.type != commonHead::viewModels::model::JsonNodeType::Object && parentData.type != commonHead::viewModels::model::JsonNodeType::Array)
            return false;

        std::size_t parentChildCount = parent->getChildCount();
        if (parentChildCount < 2)
            return false;

        return static_cast<std::size_t>(index.row()) < parentChildCount - 2;
    }
    case Qt::DisplayRole:
    {
        // Fallback display: "key: value"
        QString display;
        if (!nodeData.key.empty())
        {
            display = QString::fromStdString(nodeData.key) + ": ";
        }
        display += QString::fromStdString(nodeData.displayValue);
        return display;
    }
    default:
        break;
    }

    return {};
}

Qt::ItemFlags JsonTreeItemModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex JsonTreeItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!m_tree || row < 0 || column < 0 || column >= columnCount())
    {
        return {};
    }

    auto* parentNode = nodeFromIndex(parent);
    if (!parentNode)
    {
        return {};
    }

    if (static_cast<std::size_t>(row) >= parentNode->getChildCount())
    {
        return {};
    }

    auto child = parentNode->getChild(static_cast<std::size_t>(row));
    if (!child)
    {
        return {};
    }

    return createIndex(row, column, static_cast<void*>(child.get()));
}

QModelIndex JsonTreeItemModel::parent(const QModelIndex& index) const{
    if (!m_tree || !index.isValid())
    {
        return {};
    }

    auto root = m_tree->getRoot();
    if (!root)
    {
        return {};
    }

    auto* node = nodeFromIndex(index);
    if (!node)
    {
        return {};
    }

    auto parentWeak = node->getParent();
    auto parentShared = parentWeak.lock();
    if (!parentShared)
    {
        return {};
    }

    // If parent is root, return invalid (top-level)
    if (parentShared == root)
    {
        return {};
    }

    auto grandparentWeak = parentShared->getParent();
    auto grandparent = grandparentWeak.lock();
    if (!grandparent)
    {
        return {};
    }

    const std::size_t count = grandparent->getChildCount();
    for (std::size_t i = 0; i < count; ++i)
    {
        auto child = grandparent->getChild(i);
        if (child && child.get() == parentShared.get())
        {
            return createIndex(static_cast<int>(i), 0, static_cast<void*>(parentShared.get()));
        }
    }

    return {};
}

int JsonTreeItemModel::rowCount(const QModelIndex& parent) const
{
    if (!m_tree || (parent.isValid() && parent.column() != 0))
    {
        return 0;
    }

    auto* parentNode = nodeFromIndex(parent);
    if (!parentNode)
    {
        return 0;
    }

    return static_cast<int>(parentNode->getChildCount());
}

int JsonTreeItemModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QHash<int, QByteArray> JsonTreeItemModel::roleNames() const
{
    auto roles = QAbstractItemModel::roleNames();
    roles[KeyRole]         = "nodeKey";
    roles[ValueRole]       = "nodeValue";
    roles[NodeTypeRole]    = "nodeType";
    roles[ChildCountRole]  = "childCount";
    roles[ShowCommaRole]   = "showComma";
    return roles;
}

commonHead::viewModels::model::IJsonTreeNode* JsonTreeItemModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!m_tree)
    {
        return nullptr;
    }

    auto root = m_tree->getRoot();
    if (!root)
    {
        return nullptr;
    }

    if (!index.isValid())
    {
        return root.get();
    }

    return static_cast<commonHead::viewModels::model::IJsonTreeNode*>(index.internalPointer());
}
