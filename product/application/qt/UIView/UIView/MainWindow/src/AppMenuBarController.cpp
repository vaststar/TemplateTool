#include "MainWindow/include/AppMenuBarController.h"

#include <UIFabrication/ViewModelFactory.h>
#include <UIFabrication/UIViewFactory.h>
#include <UIManager/UIManagerProvider.h>
#include <UIManager/TranslatorManager.h>
#include <AppContext/AppContext.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include "LoggerDefine/LoggerDefine.h"

#include "ContactList/include/ContactListViewController.h"

AppMenuBarController::AppMenuBarController(QObject* parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create AppMenuBarController");
}

QString AppMenuBarController::getControllerName() const
{
    return "AppMenuBarController";
}


void AppMenuBarController::initializeController(AppContext* appContext)
{
    mAppContext = appContext;
    assert(mAppContext);

    auto clientInfoVM = appContext->getViewModelFactory()->createViewModelInstance<commonHead::viewModels::IClientInfoViewModel>();
    auto res = clientInfoVM->getSupportedLanguages();
    createMenu();
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
    mRootMenu = new MenuItemModel("Main Menu", "Root action", this);

    // 创建子菜单项
    auto fileMenu = new MenuItemModel("File", "File operations", mRootMenu);
    auto fileMenu_openFileItem = new MenuItemModel("Open", "Open a file", fileMenu);
    auto fileMenu_saveItem = new MenuItemModel("Save", "Save the file", fileMenu);
    fileMenu->addSubItem(fileMenu_openFileItem);
    fileMenu->addSubItem(fileMenu_saveItem);

    
    auto editMenu = new MenuItemModel("Edit", "Edit operations");
    auto editMenu_cutItem = new MenuItemModel("Cut", "Cut the selection", editMenu);
    auto editMenu_copyItem = new MenuItemModel("Copy", "Copy the selection", editMenu);
    auto editMenu_pasteItem = new MenuItemModel("Paste", "Paste from clipboard", editMenu);

    editMenu->addSubItem(editMenu_cutItem);
    editMenu->addSubItem(editMenu_copyItem);
    editMenu->addSubItem(editMenu_pasteItem);

    // 添加子菜单到顶级菜单
    mRootMenu->addSubItem(fileMenu);
    mRootMenu->addSubItem(editMenu);

    QObject::connect(fileMenu_openFileItem, &MenuItemModel::triggered, std::bind(&AppMenuBarController::onMenuItemTriggered, this, 0));
    QObject::connect(fileMenu_saveItem, &MenuItemModel::triggered, std::bind(&AppMenuBarController::onMenuItemTriggered, this, 1));
    QObject::connect(editMenu_cutItem, &MenuItemModel::triggered, std::bind(&AppMenuBarController::onMenuItemTriggered, this, 2));
    QObject::connect(editMenu_copyItem, &MenuItemModel::triggered, std::bind(&AppMenuBarController::onMenuItemTriggered, this, 3));
    QObject::connect(editMenu_pasteItem, &MenuItemModel::triggered, std::bind(&AppMenuBarController::onMenuItemTriggered, this, 4));

}

void AppMenuBarController::onMenuItemTriggered(int itemIndex)
{
    UIVIEW_LOG_DEBUG("item clicked:" << itemIndex);
    switchLanguage(UIManager::LanguageType::CHINESE_SIMPLIFIED);

}
