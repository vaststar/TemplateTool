#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include <UICore/CoreController.h>

namespace commonHead::viewModels
{
    class IContactListViewModel;
} // namespace name

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class AppContext;
class ContactListViewController: public UICore::CoreController
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit ContactListViewController(QObject *parent = nullptr);
    virtual QString getControllerName() const override;
    void initializeController(QPointer<AppContext> appContext);
public slots:
    void buttonClicked();
private:
    QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IContactListViewModel> mContactListViewModel;
};