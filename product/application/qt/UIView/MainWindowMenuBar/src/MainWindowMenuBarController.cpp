#include "MainWindowMenuBar/include/MainWindowMenuBarController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>

#include <UIDataStruct/UILanguage.h>
#include <UIFabrication/IViewModelFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIManager/ITranslatorManager.h>
#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

MainWindowMenuBarController::MainWindowMenuBarController(QObject* parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create MainWindowMenuBarController");
}

QString MainWindowMenuBarController::getControllerName() const
{
    return "MainWindowMenuBarController";
}


void MainWindowMenuBarController::initializeController(QPointer<AppContext> appContext)
{
    UIVIEW_LOG_DEBUG("");
    mAppContext = appContext;
    assert(mAppContext);

    auto clientInfoVM = appContext->getViewModelFactory()->createClientInfoViewModelInstance();
    auto res = clientInfoVM->getSupportedLanguages();
    createMenu();
    emit controllerInitialized();
}

void MainWindowMenuBarController::switchLanguage(UILanguage::LanguageType languageType)
{
    mAppContext->getManagerProvider()->getTranslatorManager()->loadTranslation(languageType);
}

void MainWindowMenuBarController::createMenu()
{
    // 创建顶级菜单
    mRootMenu = new MenuItemModel("Main Menu", "Root action", this);

    // 创建子菜单项
    auto fileMenu = new MenuItemModel("File", "File operations", mRootMenu);
    auto fileMenu_openFileItem = new MenuItemModel("Open", "Open a file", fileMenu);
    auto fileMenu_saveItem = new MenuItemModel("Save", "Save the file", fileMenu);
    fileMenu->addSubItem(fileMenu_openFileItem);
    fileMenu->addSubItem(fileMenu_saveItem);

    
    auto editMenu = new MenuItemModel("Edit", "Edit operations", mRootMenu);
    auto editMenu_cutItem = new MenuItemModel("Cut", "Cut the selection", editMenu);
    auto editMenu_copyItem = new MenuItemModel("Copy", "Copy the selection", editMenu);
    auto editMenu_pasteItem = new MenuItemModel("Paste", "Paste from clipboard", editMenu);

    editMenu->addSubItem(editMenu_cutItem);
    editMenu->addSubItem(editMenu_copyItem);
    editMenu->addSubItem(editMenu_pasteItem);

    // 添加子菜单到顶级菜单
    mRootMenu->addSubItem(fileMenu);
    mRootMenu->addSubItem(editMenu);

    QObject::connect(fileMenu_openFileItem, &MenuItemModel::triggered, std::bind(&MainWindowMenuBarController::onMenuItemTriggered, this, 0));
    QObject::connect(fileMenu_saveItem, &MenuItemModel::triggered, std::bind(&MainWindowMenuBarController::onMenuItemTriggered, this, 1));
    QObject::connect(editMenu_cutItem, &MenuItemModel::triggered, std::bind(&MainWindowMenuBarController::onMenuItemTriggered, this, 2));
    QObject::connect(editMenu_copyItem, &MenuItemModel::triggered, std::bind(&MainWindowMenuBarController::onMenuItemTriggered, this, 3));
    QObject::connect(editMenu_pasteItem, &MenuItemModel::triggered, std::bind(&MainWindowMenuBarController::onMenuItemTriggered, this, 4));

}

void MainWindowMenuBarController::onMenuItemTriggered(int itemIndex)
{
    UIVIEW_LOG_DEBUG("item clicked:" << itemIndex);
    switchLanguage(UILanguage::LanguageType::LanguageType_CHINESE_SIMPLIFIED);

}
