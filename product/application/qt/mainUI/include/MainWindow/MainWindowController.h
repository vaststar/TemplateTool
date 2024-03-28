#pragma once

#include <QString>
#include <QObject>

class ICommonHeadFramework;
using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

class MainWindowController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString mControllerName READ getControllerName NOTIFY controllerNameChanged)
public:
    MainWindowController(ICommonHeadFrameworkWPtr commonHeadFramework);
    Q_INVOKABLE QString getControllerName() const;
signals:
    void controllerNameChanged();
private:
    QString mControllerName;
    ICommonHeadFrameworkWPtr mCommonHeadFramework;
};