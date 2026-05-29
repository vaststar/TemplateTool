#include "CameraMonitorView/CameraDirectoryItemModel.h"

#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryTreeModel.h>

CameraDirectoryItemModel::CameraDirectoryItemModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

CameraDirectoryItemModel::~CameraDirectoryItemModel() = default;

void CameraDirectoryItemModel::setTree(
    const std::shared_ptr<commonHead::viewModels::model::ICameraDirectoryTree>& tree)
{
    beginResetModel();
    mTree = tree;
    endResetModel();
}

QVariant CameraDirectoryItemModel::data(const QModelIndex& index, int role) const
{
    if (!mTree || !index.isValid())
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
    case Qt::DisplayRole:
    case DisplayNameRole:
        return QString::fromStdString(nodeData.displayName);
    case IdRole:
        return QString::fromStdString(nodeData.id);
    case NodeTypeRole:
        return (nodeData.type ==
                commonHead::viewModels::model::CameraDirectoryNodeType::Group) ? 0 : 1;
    case StatusRole:
        return static_cast<int>(nodeData.status);
    default:
        return {};
    }
}

Qt::ItemFlags CameraDirectoryItemModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
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
    if (!mTree || row < 0 || column < 0 || column >= columnCount())
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

QModelIndex CameraDirectoryItemModel::parent(const QModelIndex& index) const
{
    if (!mTree || !index.isValid())
    {
        return {};
    }
    auto root = mTree->getRoot();
    if (!root)
    {
        return {};
    }
    auto* node = nodeFromIndex(index);
    if (!node)
    {
        return {};
    }
    auto parentShared = node->getParent().lock();
    if (!parentShared || parentShared == root)
    {
        return {};
    }
    auto grandParent = parentShared->getParent().lock();
    if (!grandParent)
    {
        return {};
    }
    const auto count = grandParent->getChildCount();
    int row = -1;
    for (std::size_t i = 0; i < count; ++i)
    {
        auto child = grandParent->getChild(i);
        if (child && child.get() == parentShared.get())
        {
            row = static_cast<int>(i);
            break;
        }
    }
    if (row < 0)
    {
        return {};
    }
    return createIndex(row, 0, static_cast<void*>(parentShared.get()));
}

int CameraDirectoryItemModel::rowCount(const QModelIndex& parent) const
{
    if (!mTree)
    {
        return 0;
    }
    if (parent.isValid() && parent.column() != 0)
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

commonHead::viewModels::model::ICameraDirectoryTreeNode*
CameraDirectoryItemModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!mTree)
    {
        return nullptr;
    }
    if (!index.isValid())
    {
        auto root = mTree->getRoot();
        return root ? root.get() : nullptr;
    }
    return static_cast<commonHead::viewModels::model::ICameraDirectoryTreeNode*>(
        index.internalPointer());
}
