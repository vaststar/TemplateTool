#pragma once

#include <QAbstractItemModel>
#include <QtQml>

#include <commonHead/viewModels/ContactListViewModel/ContactListModel.h>

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

    void setupModelData(const std::vector<commonHead::viewModels::model::Contact>& contacts);
private:
    std::vector<commonHead::viewModels::model::Contact> mContacts;
};