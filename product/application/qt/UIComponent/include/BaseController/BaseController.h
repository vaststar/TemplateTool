#pragma once

#include "UIComponentExport.h"
#include <QObject>
#include <QPointer>
class ICommonHeadFramework;
using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
using ICommonHeadFrameworkPtr = std::shared_ptr<ICommonHeadFramework>;

class UICOMPONENT_EXPORT BaseController: public QObject
{
    Q_OBJECT
public:
    BaseController(QObject *parent = nullptr);

    virtual void initController(ICommonHeadFrameworkWPtr commonheadFramework) = 0;

    Q_INVOKABLE void registerController(BaseController* controller);
    Q_INVOKABLE void setCommonHeadFramework(ICommonHeadFrameworkWPtr commonheadFramework);
    ICommonHeadFrameworkWPtr getCommonHeadFramework() const;
private:
    ICommonHeadFrameworkWPtr mCommonHeadFrameworkWPtr;
};