#include "MainWindow/include/AppMenuBarController.h"

#include <UIFabrication/ViewModelFactory.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIManager/UIManagerProvider.h>
#include <UIManager/TranslatorManager.h>
#include <AppContext/AppContext.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>
#include "LoggerDefine/LoggerDefine.h"

#include "ContactList/include/ContactListViewController.h"

AppMenuBarController::AppMenuBarController(QObject* parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create AppMenuBarController");
    createMenu();
}

QString AppMenuBarController::getControllerName() const
{
    return "AppMenuBarController";
}


void AppMenuBarController::initializeController(AppContext* appContext)
{
    mAppContext = appContext;
    assert(mAppContext);

    emit controllerInitialized();
}

void AppMenuBarController::onContactListLoaded(ContactListViewController* contactListController)
{
    contactListController->initializeController(mAppContext);
}

void AppMenuBarController::switchLanguage(UIManager::LanguageType languageType)
{
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(languageType);
}

void AppMenuBarController::createMenu()
{
    // 创建顶级菜单
    mRootMenu = new MenuItemModel("Main Menu", "Root action");

    // 创建子菜单项
    auto fileMenu = new MenuItemModel("File", "File operations");
    fileMenu->addSubItem(new MenuItemModel("Open", "Open a file"));
    fileMenu->addSubItem(new MenuItemModel("Save", "Save the file"));

    auto editMenu = new MenuItemModel("Edit", "Edit operations");
    editMenu->addSubItem(new MenuItemModel("Cut", "Cut the selection"));
    editMenu->addSubItem(new MenuItemModel("Copy", "Copy the selection"));
    editMenu->addSubItem(new MenuItemModel("Paste", "Paste from clipboard"));

    // 添加子菜单到顶级菜单
    mRootMenu->addSubItem(fileMenu);
    mRootMenu->addSubItem(editMenu);
}
