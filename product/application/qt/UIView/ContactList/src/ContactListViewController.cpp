#include "ContactList/include/ContactListViewController.h"
#include "LoggerDefine/LoggerDefine.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/ContactListModel.h>

#include <AppContext/AppContext.h>
#include <UIFabrication/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>

ContactListViewController::ContactListViewController(QObject *parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create ContactListViewController");
}

QString ContactListViewController::getControllerName() const
{
    return QObject::tr("ContactListViewController");
}

void ContactListViewController::initializeController(QPointer<AppContext> appContext)
{
    UIVIEW_LOG_DEBUG("");
    mAppContext = appContext;
    mContactListViewModel = appContext->getViewModelFactory()->createContactListViewModelInstance();
}

void ContactListViewController::buttonClicked()
{
    // mContactListViewModel->getContactList();
    // mAppContext->getViewFactory()->loadQmlWindow(QStringLiteral("UTComponent/UTWindow/UTDialog.qml"));
    mAppContext->getViewFactory()->loadQmlWindow(QStringLiteral("UTComponent/UTWindow/UTWindow.qml"));
    // mAppContext->getViewFactory()->createQmlWindow(QStringLiteral("UTComponent/UTWindow/UTTest.qml"))->show();
    
}