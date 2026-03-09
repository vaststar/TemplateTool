#include "PageViews/ToolsPage/include/ToolsTreeModel.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>

ToolsTreeModel::ToolsTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

ToolsTreeModel::~ToolsTreeModel()
{
}

void ToolsTreeModel::setTree(const std::shared_ptr<commonHead::viewModels::model::IToolsTree>& tree)
{
    if (m_tree == tree) {
        return;
    }
    beginResetModel();
    m_tree = tree;
    endResetModel();
}

QVariant ToolsTreeModel::data(const QModelIndex& index, int role) const
{
    if (!m_tree || !index.isValid()) {
        return {};
    }

    auto* node = nodeFromIndex(index);
    if (!node) {
        return {};
    }

    auto nodeData = node->getNodeData();

    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return QString::fromStdString(nodeData.title);
    case NodeIdRole:
        return QString::fromStdString(nodeData.nodeId);
    case IconRole:
        return QString::fromStdString(nodeData.icon);
    case PanelTypeRole:
        return static_cast<int>(nodeData.panelType);
    default:
        break;
    }

    return {};
}

Qt::ItemFlags ToolsTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex ToolsTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!m_tree || row < 0 || column < 0 || column >= columnCount()) {
        return {};
    }

    auto* parentNode = nodeFromIndex(parent);
    if (!parentNode) {
        return {};
    }

    if (static_cast<std::size_t>(row) >= parentNode->getChildCount()) {
        return {};
    }

    auto child = parentNode->getChild(static_cast<std::size_t>(row));
    if (!child) {
        return {};
    }

    return createIndex(row, column, static_cast<void*>(child.get()));
}

QModelIndex ToolsTreeModel::parent(const QModelIndex& index) const
{
    if (!m_tree || !index.isValid()) {
        return {};
    }

    auto root = m_tree->getRoot();
    if (!root) {
        return {};
    }

    auto* node = nodeFromIndex(index);
    if (!node) {
        return {};
    }

    auto parentWeak = node->getParent();
    auto parentShared = parentWeak.lock();
    if (!parentShared) {
        return {};
    }

    if (parentShared == root) {
        return {};
    }

    auto grandparentWeak = parentShared->getParent();
    auto grandparent = grandparentWeak.lock();
    if (!grandparent) {
        return {};
    }

    const std::size_t count = grandparent->getChildCount();
    for (std::size_t i = 0; i < count; ++i) {
        auto child = grandparent->getChild(i);
        if (child && child.get() == parentShared.get()) {
            return createIndex(static_cast<int>(i), 0, static_cast<void*>(parentShared.get()));
        }
    }

    return {};
}

int ToolsTreeModel::rowCount(const QModelIndex& parent) const
{
    if (!m_tree || (parent.isValid() && parent.column() != 0)) {
        return 0;
    }

    auto* parentNode = nodeFromIndex(parent);
    if (!parentNode) {
        return 0;
    }

    return static_cast<int>(parentNode->getChildCount());
}

int ToolsTreeModel::columnCount(const QModelIndex&) const
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

commonHead::viewModels::model::IToolsTreeNode*
ToolsTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!m_tree) {
        return nullptr;
    }

    auto root = m_tree->getRoot();
    if (!root) {
        return nullptr;
    }

    if (!index.isValid()) {
        return root.get();
    }

    return static_cast<commonHead::viewModels::model::IToolsTreeNode*>(index.internalPointer());
}
