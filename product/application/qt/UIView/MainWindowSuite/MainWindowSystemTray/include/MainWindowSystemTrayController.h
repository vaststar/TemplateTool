#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include "UIViewCommon/UIViewBase/include/UIViewController.h"

class AppContext;
class MainWindowSystemTrayController: public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    QML_ELEMENT
public:
    explicit MainWindowSystemTrayController(QObject *parent = nullptr);
    QString getTitle() const;

    Q_INVOKABLE void quit();
    Q_INVOKABLE void activateMainWindow();
protected:
    void init() override;
signals:
    void titleChanged();
};
