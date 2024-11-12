#include "MainWindow/include/MainWindowController.h"

#include <UIFabrication/ViewModelFactory.h>
#include <UIFabrication/UIViewFactory.h>
#include <AppContext/AppContext.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>
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


void MainWindowController::initializeController(AppContext* appContext)
{
    mAppContext = appContext;
    mMainViewModel = appContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IMainWindowViewModel>();
    mMainViewModel->initDatabase();
    mTitle = QObject::tr("mywindow");
    mButtonText = QObject::tr("mybutton");
            //  mAppContext->getViewFactory()->installTranslation({});
             emit titleChanged();
    emit controllerInitialized();
}

QString MainWindowController::getTitle() const
{
    return mTitle;
}

QString MainWindowController::getButton() const
{
    return mButtonText;
}
void MainWindowController::onContactListLoaded(ContactListViewController* contactListController)
{
    contactListController->initializeController(mAppContext);
}

void MainWindowController::openCamera()
{
            mAppContext->getViewFactory()->installTranslation({});
    mButtonText = QObject::tr("newmybutton");
            emit titleChanged();
    //mMainViewModel->openCamera();
}
