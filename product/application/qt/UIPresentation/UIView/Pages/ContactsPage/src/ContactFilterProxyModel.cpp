#include "ContactsPage/ContactFilterProxyModel.h"

#include "ContactsPage/ContactListItemModel.h"

ContactFilterProxyModel::ContactFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setRecursiveFilteringEnabled(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterRole(Qt::DisplayRole);
}

void ContactFilterProxyModel::setSearchText(const QString& text)
{
    setFilterFixedString(text.trimmed());
}

QModelIndex ContactFilterProxyModel::indexOfId(const QString& id) const
{
    auto* source = qobject_cast<ContactListItemModel*>(sourceModel());
    if (!source)
    {
        return {};
    }

    return mapFromSource(source->indexOfId(id));
}
