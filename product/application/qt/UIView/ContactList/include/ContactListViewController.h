#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

namespace commonHead::viewModels
{
    class IContactListViewModel;
} // namespace name

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class ContactListViewController: public UIViewController
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit ContactListViewController(QObject *parent = nullptr);
public slots:
    void buttonClicked();
protected:
    void init() override;
private:
    std::shared_ptr<commonHead::viewModels::IContactListViewModel> mContactListViewModel;
};