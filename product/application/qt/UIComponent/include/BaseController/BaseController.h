#pragma once

#include "UIComponentExport.h"
#include <QObject>
#include <QPointer>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
    using ICommonHeadFrameworkPtr = std::shared_ptr<ICommonHeadFramework>;
}
class UICOMPONENT_EXPORT BaseController: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString mControllerName READ getControllerName NOTIFY controllerNameChanged)
public:
    BaseController(QObject *parent = nullptr);
    virtual ~BaseController() = default;
    virtual QString getControllerName() const = 0;

    void setCommonHeadFramework(commonHead::ICommonHeadFrameworkWPtr commonheadFramework);
    commonHead::ICommonHeadFrameworkWPtr getCommonHeadFramework() const;
signals:
    void controllerNameChanged();
private:
    commonHead::ICommonHeadFrameworkWPtr mCommonHeadFrameworkWPtr;
};