#include "ContactList/include/ContactListItemModel.h"

#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>

enum Roles {
    IdRole = Qt::UserRole + 1,
    DisplayNameRole,
    TypeRole  // 0 = Person, 1 = Group
};

ContactListItemModel::ContactListItemModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

ContactListItemModel::~ContactListItemModel()
{
    
}

QVariant ContactListItemModel::data(const QModelIndex &index, int role) const
{
    if (!mViewModel || !index.isValid()) {
        return {};
    }

    commonHead::viewModels::model::IContactTreeNode* node = nodeFromIndex(index);
    if (!node) {
        return {};
    }

    commonHead::viewModels::model::ContactNodeData nodeData = node->getNodeData();

    switch (role) {
    case Qt::DisplayRole:
    case DisplayNameRole:
        return QString::fromStdString(nodeData.displayName);
    case IdRole:
        return QString::fromStdString(nodeData.id);
    case TypeRole:
        return (nodeData.type ==
                commonHead::viewModels::model::ContactNodeType::Person) ? 0 : 1;
    default:
        break;
    }

    return {};
}

Qt::ItemFlags ContactListItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ContactListItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return QStringLiteral("Contact");
        }
    }
    return {};
}

QModelIndex ContactListItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!mViewModel) {
        return {};
    }
    if (row < 0 || column < 0 || column >= columnCount()) {
        return {};
    }
    
    commonHead::viewModels::model::IContactTreeNode* parentNode = nodeFromIndex(parent);
    if (!parentNode) {
        return {};
    }

    if (static_cast<std::size_t>(row) >= parentNode->getChildCount()) {
        return {};
    }

    std::shared_ptr<commonHead::viewModels::model::IContactTreeNode> child =
        parentNode->getChild(static_cast<std::size_t>(row));

    if (!child) {
        return {};
    }

    return createIndex(
        row,
        column,
        static_cast<void*>(child.get())
    );
}

QModelIndex ContactListItemModel::parent(const QModelIndex &index) const
{
     if (!mViewModel || !index.isValid()) {
        return {};
    }

    commonHead::viewModels::model::ContactTreePtr tree = mViewModel->getContactList();
    if (!tree) {
        return {};
    }

    commonHead::viewModels::model::IContactTreeNode* node = nodeFromIndex(index);
    if (!node) {
        return {};
    }

    std::weak_ptr<commonHead::viewModels::model::IContactTreeNode> parentWeak = node->getParent();
    std::shared_ptr<commonHead::viewModels::model::IContactTreeNode> parentShared = parentWeak.lock();

    if (!parentShared) {
        return {};
    }

    std::shared_ptr<commonHead::viewModels::model::IContactTreeNode> root = tree->getRoot();
    if (!root) {
        return {};
    }

    if (parentShared == root) {
        return {};
    }

    std::weak_ptr<commonHead::viewModels::model::IContactTreeNode> gWeak = parentShared->getParent();
    std::shared_ptr<commonHead::viewModels::model::IContactTreeNode> grandParent = gWeak.lock();

    if (!grandParent) {
        return {};
    }

    const std::size_t count = grandParent->getChildCount();
    int row = -1;

    for (std::size_t i = 0; i < count; ++i) {
        std::shared_ptr<commonHead::viewModels::model::IContactTreeNode> child =
            grandParent->getChild(i);

        if (child && child.get() == parentShared.get()) {
            row = static_cast<int>(i);
            break;
        }
    }

    if (row < 0) {
        return {};
    }

    return createIndex(
        row,
        0,
        static_cast<void*>(parentShared.get())
    );
}

int ContactListItemModel::rowCount(const QModelIndex &parent) const
{
    if (!mViewModel) {
        return 0;
    }

    if (parent.isValid() && parent.column() != 0) {
        return 0;
    }

    commonHead::viewModels::model::ContactTreePtr tree = mViewModel->getContactList();
    if (!tree) {
        return 0;
    }

    commonHead::viewModels::model::IContactTreeNode* parentNode = nodeFromIndex(parent);
    if (!parentNode) {
        return 0;
    }

    return static_cast<int>(parentNode->getChildCount());
}

int ContactListItemModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QHash<int, QByteArray> ContactListItemModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole]          = "id";
    roles[DisplayNameRole] = "displayName";
    roles[TypeRole]        = "nodeType";
    return roles;
}

commonHead::viewModels::model::IContactTreeNode*
ContactListItemModel::nodeFromIndex(const QModelIndex &index) const
{
    if (!mViewModel) {
        return nullptr;
    }

    commonHead::viewModels::model::ContactTreePtr tree = mViewModel->getContactList();
    if (!tree) {
        return nullptr;
    }

    std::shared_ptr<commonHead::viewModels::model::IContactTreeNode> root = tree->getRoot();
    if (!root) {
        return nullptr;
    }

    if (!index.isValid()) {
        return root.get();
    }

    return static_cast<commonHead::viewModels::model::IContactTreeNode*>(index.internalPointer());
}

void ContactListItemModel::setUpViewModel(const std::shared_ptr<commonHead::viewModels::IContactListViewModel>& viewModel)
{
    if (mViewModel == viewModel)
    {
        return;
    }
    beginResetModel();
    mViewModel = viewModel;
    endResetModel();
}
