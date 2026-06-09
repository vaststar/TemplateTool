#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVariant>

#include <commonHead/viewModels/SideBarViewModel/SideBarModel.h>

// List model that owns a UI-layer mirror of nav items. VM data is converted
// at the boundary (fromVM); data() reads only the internal Item struct.
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
        SortOrderRole,
        IsEnabledRole,
        IsVisibleRole,
        HasSubMenuRole
    };

    explicit NavItemModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Mutation API. Takes VM data; conversion to Item happens internally.
    void resetItems  (const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void insertItems (const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void updateItems (const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void removeItems (const std::vector<commonHead::viewModels::model::PageId>&     pageIds);

    bool contains(int pageId) const;

private:
    // UI-layer mirror. Icons are pre-converted so data() is trivial.
    struct Item
    {
        int      pageId      = 0;
        QString  itemId;
        QString  title;
        QVariant icon;
        QVariant iconSelected;
        int      badge       = 0;
        int      state       = 0;
        int      position    = 0;
        int      sortOrder   = 0;
        bool     isEnabled   = true;
        bool     isVisible   = true;
        bool     hasSubMenu  = false;
    };

    static Item fromVM(const commonHead::viewModels::model::NavItemData& vm);
    int rowOfPageId(int pageId) const;

private:
    std::vector<Item> m_items;
};
