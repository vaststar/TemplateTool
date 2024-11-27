#pragma once

#include <QObject>
#include <QtQml>
#include <QString>
#include <QList>
#include <include/UTComponentExport.h>

class UTCOMPONENT_EXPORT MenuItemModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString action READ action CONSTANT)
    Q_PROPERTY(QList<MenuItemModel*> subItems READ subItems CONSTANT)
    QML_ELEMENT
public:
    MenuItemModel(const QString& name, const QString& action, QObject* parent = nullptr);

    QString name() const;
    QString action() const;
    QList<MenuItemModel*> subItems() const;
    void addSubItem(MenuItemModel* item);
signals:
    void triggered();
private:
    QString m_name;
    QString m_action;
    QList<MenuItemModel*> m_subItems;
};