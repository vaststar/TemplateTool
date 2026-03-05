#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"
#include "ContactListItemModel.h"

namespace commonHead::viewModels
{
    class IContactListViewModel;
}

class ContactsPageController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* orgTreeModel READ getOrgTreeModel NOTIFY orgTreeModelChanged)
    QML_ELEMENT

public:
    explicit ContactsPageController(QObject* parent = nullptr);
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
