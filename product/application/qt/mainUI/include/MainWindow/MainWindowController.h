#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include "BaseController/BaseController.h"

class ICommonHeadFramework;
using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

class MainWindowController : public BaseController
{
    Q_OBJECT
    Q_PROPERTY(QString mControllerName READ getControllerName NOTIFY controllerNameChanged)
    QML_ELEMENT
public:
    MainWindowController(QObject* parent =nullptr);
    QString getControllerName() const;
    virtual void initController(ICommonHeadFrameworkWPtr commonheadFramework) override;
signals:
    void controllerNameChanged();
private:
    QString mControllerName;
    ICommonHeadFrameworkWPtr mCommonHeadFramework;
};