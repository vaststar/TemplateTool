#include "PageViews/SettingsPage/include/SettingsTreeModel.h"

#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>
#include <UIResourceAssetLoader/UIResourceAssetLoader.h>

SettingsTreeModel::SettingsTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

SettingsTreeModel::~SettingsTreeModel()
{
}

void SettingsTreeModel::setTree(const std::shared_ptr<commonHead::viewModels::model::ISettingsTree>& tree)
{
    if (m_tree == tree) {
        return;
    }
    beginResetModel();
    m_tree = tree;
    endResetModel();
}

QVariant SettingsTreeModel::data(const QModelIndex& index, int role) const
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
        return QVariant::fromValue(
            UIResource::UIResourceAssetLoader::convertVMAssetImageTokenToUIAssetImageToken(nodeData.icon)
        );
    case PanelTypeRole:
        return static_cast<int>(nodeData.panelType);
    default:
        break;
    }

    return {};
}

Qt::ItemFlags SettingsTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex SettingsTreeModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex SettingsTreeModel::parent(const QModelIndex& index) const
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

int SettingsTreeModel::rowCount(const QModelIndex& parent) const
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

int SettingsTreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QHash<int, QByteArray> SettingsTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NodeIdRole]    = "nodeId";
    roles[TitleRole]     = "title";
    roles[IconRole]      = "icon";
    roles[PanelTypeRole] = "panelType";
    return roles;
}

commonHead::viewModels::model::ISettingsTreeNode*
SettingsTreeModel::nodeFromIndex(const QModelIndex& index) const
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

    return static_cast<commonHead::viewModels::model::ISettingsTreeNode*>(index.internalPointer());
}

QModelIndex SettingsTreeModel::indexForNodeId(const std::string& nodeId) const
{
    if (!m_tree) return {};

    auto node = m_tree->findNodeById(nodeId);
    if (!node) return {};

    auto root = m_tree->getRoot();
    if (node == root) return {};

    auto parent = node->getParent().lock();
    if (!parent) return {};

    for (std::size_t i = 0; i < parent->getChildCount(); ++i) {
        if (parent->getChild(i).get() == node.get()) {
            return createIndex(static_cast<int>(i), 0,
                               static_cast<void*>(node.get()));
        }
    }
    return {};
}

void SettingsTreeModel::notifyItemChanged(const std::string& nodeId)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (idx.isValid()) {
        emit dataChanged(idx, idx);
    }
}

void SettingsTreeModel::notifyAllItemsChanged()
{
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void SettingsTreeModel::applyStructureChange(
    const commonHead::viewModels::model::SettingsTreeNodeChange& change)
{
    int row = static_cast<int>(change.index);
    QModelIndex parentIdx = indexForNodeId(change.parentNodeId);

    switch (change.type) {
    case commonHead::viewModels::model::SettingsTreeNodeChange::Type::Inserted:
        // Data already added to tree by ViewModel
        beginInsertRows(parentIdx, row, row);
        endInsertRows();
        break;
    case commonHead::viewModels::model::SettingsTreeNodeChange::Type::Removed:
        // Data not yet removed — ViewModel removes after this returns
        beginRemoveRows(parentIdx, row, row);
        endRemoveRows();
        break;
    }
}
