#pragma once

#include <QObject>
#include <QtQml>
#include "BaseController/BaseController.h"

namespace commonHead::viewModels
{
    class IContactListViewModel;
} // namespace name

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class ContactListViewController: public BaseController
{
    Q_OBJECT
    // Q_PROPERTY(QString mControllerName READ getControllerName)
    QML_ELEMENT
public:
    explicit ContactListViewController(QObject *parent = nullptr);
    virtual QString getControllerName() const override;
public slots:
    void buttonClicked();
private:
    // QString mControllerName;
    std::shared_ptr<commonHead::viewModels::IContactListViewModel> mContactListViewModel;
};