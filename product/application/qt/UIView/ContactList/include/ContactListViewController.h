#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"
#include "ContactListItemModel.h"

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
    Q_PROPERTY(QAbstractItemModel* orgTreeModel READ getOrgTreeModel NOTIFY orgTreeModelChanged)
    QML_ELEMENT
public:
    explicit ContactListViewController(QObject *parent = nullptr);
    QAbstractItemModel* getOrgTreeModel() const;
public slots:
    void buttonClicked();
protected:
    void init() override;
private:
    void buildContactTreeModel();
signals:
    void orgTreeModelChanged();
private:
    std::shared_ptr<commonHead::viewModels::IContactListViewModel> mContactListViewModel;
    ContactListItemModel* mOrgTreeModel = nullptr;
};