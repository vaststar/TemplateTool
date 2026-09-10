#pragma once

#include <QSortFilterProxyModel>
#include <QString>

class ContactFilterProxyModel final : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ContactFilterProxyModel(QObject* parent = nullptr);

    void setSearchText(const QString& text);

    // UTTreeView locates nodes by id after add/move callbacks. Return an index
    // belonging to this proxy model rather than leaking a source-model index.
    Q_INVOKABLE QModelIndex indexOfId(const QString& id) const;
};
