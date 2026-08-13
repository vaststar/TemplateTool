#include "MainWindowSideBar/NavItemModel.h"

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
        case PageIdRole:        return item.pageId;
        case IdRole:            return item.itemId;
        case TitleRole:         return item.title;
        case IconRole:          return item.icon;
        case IconSelectedRole:  return item.iconSelected;
        case BadgeRole:         return item.badge;
        case StateRole:         return item.state;
        case PositionRole:      return item.position;
        case SortOrderRole:     return item.sortOrder;
        case IsEnabledRole:     return item.isEnabled;
        case IsVisibleRole:     return item.isVisible;
        case HasSubMenuRole:    return item.hasSubMenu;
        default:                return {};
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
        {SortOrderRole, "sortOrder"},
        {IsEnabledRole, "isEnabled"},
        {IsVisibleRole, "isVisible"},
        {HasSubMenuRole, "hasSubMenu"}
    };
}

NavItemModel::Item NavItemModel::fromVM(const commonHead::viewModels::model::NavItemData& vm)
{
    Item out;
    out.pageId       = static_cast<int>(vm.pageId);
    out.itemId       = QString::fromStdString(vm.id);
    out.title        = QString::fromStdString(vm.title);
    out.icon         = QVariant::fromValue(
        UIResource::UIResourceAssetLoader::convertVMAssetImageTokenToUIAssetImageToken(vm.icon));
    out.iconSelected = QVariant::fromValue(
        UIResource::UIResourceAssetLoader::convertVMAssetImageTokenToUIAssetImageToken(vm.iconSelected));
    out.badge        = vm.badge;
    out.state        = static_cast<int>(vm.state);
    out.position     = static_cast<int>(vm.position);
    out.sortOrder    = vm.sortOrder;
    out.isEnabled    = vm.isEnabled();
    out.isVisible    = vm.isVisible();
    out.hasSubMenu   = vm.hasSubMenu();
    return out;
}

int NavItemModel::rowOfPageId(int pageId) const
{
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i].pageId == pageId)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool NavItemModel::contains(int pageId) const
{
    return rowOfPageId(pageId) >= 0;
}

void NavItemModel::resetItems(const std::vector<commonHead::viewModels::model::NavItemData>& items)
{
    beginResetModel();
    m_items.clear();
    m_items.reserve(items.size());
    for (const auto& vm : items)
    {
        m_items.push_back(fromVM(vm));
    }
    endResetModel();
}

void NavItemModel::insertItems(const std::vector<commonHead::viewModels::model::NavItemData>& items)
{
    if (items.empty())
    {
        return;
    }
    const int first = static_cast<int>(m_items.size());
    const int last  = first + static_cast<int>(items.size()) - 1;
    beginInsertRows(QModelIndex(), first, last);
    m_items.reserve(m_items.size() + items.size());
    for (const auto& vm : items)
    {
        m_items.push_back(fromVM(vm));
    }
    endInsertRows();
}

void NavItemModel::updateItems(const std::vector<commonHead::viewModels::model::NavItemData>& items)
{
    for (const auto& vm : items)
    {
        const int row = rowOfPageId(static_cast<int>(vm.pageId));
        if (row < 0)
        {
            continue;  // not in this model (e.g. wrong position); caller handles routing
        }
        m_items[row] = fromVM(vm);
        const QModelIndex idx = index(row);
        emit dataChanged(idx, idx);
    }
}

void NavItemModel::removeItems(const std::vector<commonHead::viewModels::model::PageId>& pageIds)
{
    for (const auto pid : pageIds)
    {
        const int row = rowOfPageId(static_cast<int>(pid));
        if (row < 0)
        {
            continue;
        }
        beginRemoveRows(QModelIndex(), row, row);
        m_items.erase(m_items.begin() + row);
        endRemoveRows();
    }
}
