#include "MainWindowSideBar/include/NavItemModel.h"

#include <UIResourceAssetLoader/UIResourceAssetLoader.h>

NavItemModel::NavItemModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int NavItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(m_items.size());
}

QVariant NavItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_items.size()))
    {
        return {};
    }

    const auto& item = m_items[index.row()];

    switch (role)
    {
        case PageIdRole:
            return static_cast<int>(item.pageId);
        case IdRole:
            return QString::fromStdString(item.id);
        case TitleRole:
            return QString::fromStdString(item.title);
        case IconRole:
            return QVariant::fromValue(
                UIResource::UIResourceAssetLoader::convertVMAssetImageTokenToUIAssetImageToken(item.icon)
            );
        case IconSelectedRole:
            return QVariant::fromValue(
                UIResource::UIResourceAssetLoader::convertVMAssetImageTokenToUIAssetImageToken(item.iconSelected)
            );
        case BadgeRole:
            return item.badge;
        case StateRole:
            return static_cast<int>(item.state);
        case PositionRole:
            return static_cast<int>(item.position);
        case SortOrderRole:
            return item.sortOrder;
        default:
            return {};
    }
}

QHash<int, QByteArray> NavItemModel::roleNames() const
{
    return {
        {PageIdRole, "pageId"},
        {IdRole, "itemId"},
        {TitleRole, "title"},
        {IconRole, "icon"},
        {IconSelectedRole, "iconSelected"},
        {BadgeRole, "badge"},
        {StateRole, "state"},
        {PositionRole, "position"},
        {SortOrderRole, "sortOrder"}
    };
}

void NavItemModel::setItems(const std::vector<commonHead::viewModels::model::NavItemData>& items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void NavItemModel::updateItem(const commonHead::viewModels::model::NavItemData& item)
{
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i].pageId == item.pageId)
        {
            m_items[i] = item;
            QModelIndex idx = index(static_cast<int>(i));
            emit dataChanged(idx, idx);
            return;
        }
    }
}
