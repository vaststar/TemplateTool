#include "PageViews/SettingsPage/include/SettingsNavModel.h"

SettingsNavModel::SettingsNavModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int SettingsNavModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_items.size());
}

QVariant SettingsNavModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_items.size()))
        return {};

    const auto& item = m_items[static_cast<size_t>(index.row())];
    switch (role)
    {
    case IdRole:      return item.id;
    case SectionRole: return item.section;
    case TitleRole:   return item.title;
    case IconRole:    return item.icon;
    default:          return {};
    }
}

QHash<int, QByteArray> SettingsNavModel::roleNames() const
{
    return {
        { IdRole,      "navId" },
        { SectionRole, "section" },
        { TitleRole,   "title" },
        { IconRole,    "icon" }
    };
}

void SettingsNavModel::setItems(const std::vector<SettingsNavItem>& items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}
