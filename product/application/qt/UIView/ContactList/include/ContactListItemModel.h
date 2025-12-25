#pragma once

#include <QAbstractItemModel>
#include <QtQml>

namespace commonHead::viewModels{
    class IContactListViewModel;
}

namespace commonHead::viewModels::model{
    class IContactTreeNode;
}

class ContactListItemModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    ContactListItemModel(QObject *parent = nullptr);
    ~ContactListItemModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;

    void setUpViewModel(const std::shared_ptr<commonHead::viewModels::IContactListViewModel>& viewModel);
protected:
    // 为 QML 提供角色名
    QHash<int, QByteArray> roleNames() const override;

private:
    // 帮助函数：从 QModelIndex 取出节点指针
    commonHead::viewModels::model::IContactTreeNode* nodeFromIndex(const QModelIndex& index) const;

private:
    std::shared_ptr<commonHead::viewModels::IContactListViewModel> mViewModel;
};