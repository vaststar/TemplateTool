#pragma once

#include <QString>
#include <QObject>
#include <QtQml>
#include "BaseController/BaseController.h"

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class MainWindowController : public BaseController
{
    Q_OBJECT
    // Q_PROPERTY(QString mControllerName READ getControllerName NOTIFY controllerNameChanged)
    QML_ELEMENT
public:
    MainWindowController(QObject* parent =nullptr);
    virtual QString getControllerName() const override;
signals:
    void controllerNameChanged();
};