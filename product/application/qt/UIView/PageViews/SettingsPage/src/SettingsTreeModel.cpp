#include "PageViews/SettingsPage/include/SettingsTreeModel.h"

#include <commonHead/viewModels/SettingsViewModel/ISettingsViewModel.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>
#include <UIResourceAssetLoader/UIResourceAssetLoader.h>

SettingsTreeModel::SettingsTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

SettingsTreeModel::~SettingsTreeModel()
{
}

void SettingsTreeModel::setViewModel(const std::shared_ptr<commonHead::viewModels::ISettingsViewModel>& viewModel)
{
    if (m_viewModel == viewModel) {
        return;
    }
    beginResetModel();
    m_viewModel = viewModel;
    endResetModel();
}

QVariant SettingsTreeModel::data(const QModelIndex& index, int role) const
{
    if (!m_viewModel || !index.isValid()) {
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
    if (!m_viewModel || row < 0 || column < 0 || column >= columnCount()) {
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
    if (!m_viewModel || !index.isValid()) {
        return {};
    }

    auto tree = m_viewModel->getSettingsTree();
    if (!tree) {
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

    auto root = tree->getRoot();
    if (!root || parentShared == root) {
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
    if (!m_viewModel || (parent.isValid() && parent.column() != 0)) {
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
    if (!m_viewModel) {
        return nullptr;
    }

    auto tree = m_viewModel->getSettingsTree();
    if (!tree) {
        return nullptr;
    }

    auto root = tree->getRoot();
    if (!root) {
        return nullptr;
    }

    if (!index.isValid()) {
        return root.get();
    }

    return static_cast<commonHead::viewModels::model::ISettingsTreeNode*>(index.internalPointer());
}
