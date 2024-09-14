#pragma once

#include <QObject>
#include <QtQml>
// #include "BaseController/BaseController.h"
#include <UICore/CoreController.h>
#include <UICore/CoreContext.h>

namespace commonHead::viewModels
{
    class IContactListViewModel;
} // namespace name

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class CoreContext;
class ContactListViewController: public CoreController
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit ContactListViewController(QObject *parent = nullptr);
    virtual QString getControllerName() const override;
    void initializeController(CoreContext* appContext);
public slots:
    void buttonClicked();
private:
    CoreContext* mAppContext;
    std::shared_ptr<commonHead::viewModels::IContactListViewModel> mContactListViewModel;
};