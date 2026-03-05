#pragma once

#include <QAbstractListModel>
#include <QtQml>
#include <vector>

struct SettingsNavItem
{
    int id;
    QString section;
    QString title;
    QString icon; // 保留扩展，暂不使用
};

class SettingsNavModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles
    {
        IdRole = Qt::UserRole + 1,
        SectionRole,
        TitleRole,
        IconRole
    };

    explicit SettingsNavModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const std::vector<SettingsNavItem>& items);

private:
    std::vector<SettingsNavItem> m_items;
};
