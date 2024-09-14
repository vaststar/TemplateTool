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
void MainWindowController::showMainWindow()
{
    // UIVIEW_LOG_DEBUG("start load main qml");
    // const QUrl url(QStringLiteral("qrc:/qt/qml/UIView/qml/MainWindow/MainWindow.qml"));

    // mAppContext.getViewFactory()->loadQmlWindow(QStringLiteral("qrc:/qt/qml/UIView/qml/MainWindow/MainWindow.qml"));
    // setProperty(const char *name, const QVariant &value)
    // // mImpl->getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("qrc:/qt/qml/UIView/qml/testUI/testWindow.qml"));
    // // mDataPrivate->mainApp.mApplicationEngine->load(url);
    // // mDataPrivate->mainApp.mApplicationEngine->load(url);
    // UIVIEW_LOG_DEBUG("finish load main qml");

}

void MainWindowController::onContactListLoaded(ContactListViewController* contactListController)
{
    contactListController->initializeController(mAppContext);
}