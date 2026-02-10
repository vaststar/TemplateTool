#include "ContactList/include/ContactListViewController.h"
#include "LoggerDefine/LoggerDefine.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>

ContactListViewController::ContactListViewController(QObject *parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create ContactListViewController");
}

void ContactListViewController::init()
{
    UIVIEW_LOG_DEBUG("");
    mContactListViewModel = getAppContext()->getViewModelFactory()->createContactListViewModelInstance();
    mContactListViewModel->initViewModel();
    buildContactTreeModel();
    UIVIEW_LOG_DEBUG("done");
}

void ContactListViewController::buttonClicked()
{
    // mContactListViewModel->getContactList();
    // mAppContext->getViewFactory()->loadQmlWindow(QStringLiteral("UTComponent/UTWindow/UTDialog.qml"));
    getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("UTComponent/UTWindow/UTWindow.qml"));
    // mAppContext->getViewFactory()->createQmlWindow(QStringLiteral("UTComponent/UTWindow/UTTest.qml"))->show();
    
}

QAbstractItemModel* ContactListViewController::getOrgTreeModel() const
{
    return mOrgTreeModel;
}

void ContactListViewController::buildContactTreeModel()
{
    if (!mContactListViewModel)
    {
        UIVIEW_LOG_ERROR("mContactListViewModel is null");
        return;
    }

    if (mOrgTreeModel)
    {
        delete mOrgTreeModel;
        mOrgTreeModel = nullptr;
    }

    mOrgTreeModel = new ContactListItemModel(this);
    mOrgTreeModel->setUpViewModel(mContactListViewModel);
    // mOrgTreeModel->setupModelData(mContactListViewModel->getContactList()->getAllContacts());

    emit orgTreeModelChanged();
}