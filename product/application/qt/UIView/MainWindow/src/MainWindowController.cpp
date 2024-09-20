#include "MainWindow/include/MainWindowController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include "LoggerDefine/LoggerDefine.h"

#include "ContactList/include/ContactListViewController.h"

MainWindowController::MainWindowController(QObject* parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create MainWindowController");
}

QString MainWindowController::getControllerName() const
{
    return "MainWindowController";
}


void MainWindowController::initializeController(CoreContext* appContext)
{
    mAppContext = appContext;
    mTitle = "intialized window";
    emit titleChanged();
    emit controllerInitialized(appContext);
}

QString MainWindowController::getTitle() const
{
    return mTitle;
}

void MainWindowController::onContactListLoaded(ContactListViewController* contactListController)
{
    contactListController->initializeController(mAppContext);
}