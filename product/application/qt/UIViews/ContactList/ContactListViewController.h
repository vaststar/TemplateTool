#pragma once

#include <QObject>
#include <QtQml>
#include "BaseController/BaseController.h"

namespace CommonHead::ViewModels
{
    class IContactListViewModel;
} // namespace name
class ICommonHeadFramework;
using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

class ContactListViewController: public BaseController
{
    Q_OBJECT
    Q_PROPERTY(QString mControllerName READ getControllerName)
    QML_ELEMENT
public:
    explicit ContactListViewController(QObject *parent = nullptr);
    QString getControllerName();
    virtual void initController(ICommonHeadFrameworkWPtr commonheadFramework) override;
private:
    QString mControllerName;
};