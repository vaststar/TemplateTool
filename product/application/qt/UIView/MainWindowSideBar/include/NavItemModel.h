#pragma once

#include <QAbstractListModel>
#include <commonHead/viewModels/SideBarViewModel/SideBarModel.h>

class NavItemModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        PageIdRole = Qt::UserRole + 1,
        IdRole,
        TitleRole,
        IconRole,
        IconSelectedRole,
        BadgeRole,
        StateRole,
        PositionRole,
        SortOrderRole
    };

    explicit NavItemModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void updateItem(const commonHead::viewModels::model::NavItemData& item);

private:
    std::vector<commonHead::viewModels::model::NavItemData> m_items;
};
