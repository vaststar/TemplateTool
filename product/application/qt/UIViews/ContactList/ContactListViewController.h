#pragma once

#include <QObject>
#include <QtQml>

class ContactListViewController: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString mControllerName READ getControllerName)
    QML_ELEMENT
public:
    explicit ContactListViewController(QObject *parent = nullptr);
    QString getControllerName();
private:
    QString mControllerName;
};