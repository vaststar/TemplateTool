#include "ContactList/ContactListViewController.h"
#include "LoggerDefine.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/ContactListModel.h>

#include <UICore/CoreContext.h>
#include <UICore/CoreViewModelFactory.h>
ContactListViewController::ContactListViewController(QObject *parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create ContactListViewController");
}

QString ContactListViewController::getControllerName() const
{
    return "ContactListViewController";
}

void ContactListViewController::initializeController(CoreContext* appContext)
{
    mAppContext = appContext;
    mContactListViewModel = appContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IContactListViewModel>();
}

void ContactListViewController::buttonClicked()
{
    mContactListViewModel->getContactList();
}