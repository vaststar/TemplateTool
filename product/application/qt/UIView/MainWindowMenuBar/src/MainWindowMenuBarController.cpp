#include "MainWindowMenuBar/include/MainWindowMenuBarController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>

#include <TranslatorManager/UILanguage.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

MainWindowMenuBarController::MainWindowMenuBarController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create MainWindowMenuBarController");
}

void MainWindowMenuBarController::init()
{
    UIVIEW_LOG_DEBUG("");

    auto clientInfoVM = getAppContext()->getViewModelFactory()->createClientInfoViewModelInstance();
    auto res = clientInfoVM->getSupportedLanguages();
    buildMenuModel();
    emit controllerInitialized();
}

void MainWindowMenuBarController::switchLanguage(UILanguage::LanguageType languageType)
{
    getAppContext()->getManagerProvider()->getTranslatorManager()->loadTranslation(languageType);
}

void MainWindowMenuBarController::buildMenuModel()
{
    QVariantList fileItems;
    fileItems.append(QVariantMap{{"text", "Open"}, {"action", "file_open"}});
    fileItems.append(QVariantMap{{"text", "Save"}, {"action", "file_save"}});

    QVariantList editItems;
    editItems.append(QVariantMap{{"text", "Cut"},   {"action", "edit_cut"}});
    editItems.append(QVariantMap{{"text", "Copy"},  {"action", "edit_copy"}});
    editItems.append(QVariantMap{{"text", "Paste"}, {"action", "edit_paste"}});

    m_menuModel.clear();
    m_menuModel.append(QVariantMap{{"text", "File"}, {"children", fileItems}});
    m_menuModel.append(QVariantMap{{"text", "Edit"}, {"children", editItems}});
    emit menuModelChanged();
}

void MainWindowMenuBarController::handleMenuAction(const QString& action)
{
    UIVIEW_LOG_DEBUG("menu action: " << action.toStdString());

    if (action == "file_open") {
        // TODO: open file
    } else if (action == "file_save") {
        // TODO: save file
    } else if (action == "edit_cut") {
        // TODO: cut
    } else if (action == "edit_copy") {
        // TODO: copy
    } else if (action == "edit_paste") {
        // TODO: paste
    }

    switchLanguage(UILanguage::LanguageType::LanguageType_CHINESE_SIMPLIFIED);
}
