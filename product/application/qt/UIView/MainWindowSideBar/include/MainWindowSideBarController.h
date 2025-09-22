#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>

#include "UIViewBase/include/UIViewController.h"

class AppContext;
class MainWindowSideBarController: public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    QML_ELEMENT
public:
    explicit MainWindowSideBarController(QObject *parent = nullptr);
    QString getTitle() const;
signals:
    void titleChanged();
protected:
    virtual void init() override;
};