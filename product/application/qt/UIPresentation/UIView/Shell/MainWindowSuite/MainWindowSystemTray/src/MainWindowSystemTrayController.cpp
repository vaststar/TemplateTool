#include "MainWindowSystemTray/MainWindowSystemTrayController.h"

#include <AppContext/AppContext.h>
#include <QVariantMap>

#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <commonhead/viewmodels/TrayViewModel/ITrayViewModel.h>
#include <UIViewModelSignalBridge/emitters/TrayViewModelEmitter.h>

#include "LoggerDefine.h"
#include "UIEvents/UIMainWindowEvent.h"
#include "UIEvents/UISettingsEvent.h"
#include "UIEvents/UIUpgradeEvent.h"
#include "UIEvents/UIAboutEvent.h"

MainWindowSystemTrayController::MainWindowSystemTrayController(QObject *parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIViewModelSignalBridge::TrayViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("MainWindowSystemTrayController constructed, address: " << this);
}

MainWindowSystemTrayController::~MainWindowSystemTrayController()
{
    UIVIEW_LOG_DEBUG("MainWindowSystemTrayController destroying, address: " << this);
}

QString MainWindowSystemTrayController::getTitle() const
{
    return QObject::tr("my test window title bar");
}

QVariantList MainWindowSystemTrayController::menuItems() const
{
    QVariantList items;
    if (!m_trayViewModel)
        return items;

    for (const auto& item : m_trayViewModel->getMenuItems())
    {
        QVariantMap entry;
        entry["actionId"] = static_cast<int>(item.actionId);
        entry["title"] = QString::fromStdString(item.title);
        entry["enabled"] = item.enabled;
        entry["separator"] = item.kind == commonHead::viewModels::model::TrayMenuItemKind::Separator;
        items.append(entry);
    }
    return items;
}

void MainWindowSystemTrayController::handleMenuAction(int actionId)
{
    if (m_trayViewModel)
    {
        m_trayViewModel->handleMenuAction(
            static_cast<commonHead::viewModels::model::TrayActionId>(actionId));
    }
}

void MainWindowSystemTrayController::onActionRequested(int actionId)
{
    switch (static_cast<commonHead::viewModels::model::TrayActionId>(actionId))
    {
    case commonHead::viewModels::model::TrayActionId::ShowMainWindow:
        activateMainWindow();
        break;
    case commonHead::viewModels::model::TrayActionId::OpenSettings:
        sendUIEvent<UISettingsEvent>(UISettingsEvent::Action::ShowSettings);
        break;
    case commonHead::viewModels::model::TrayActionId::CheckForUpdates:
        activateMainWindow();
        sendUIEvent<UIUpgradeEvent>(UIUpgradeEvent::Action::CheckForUpgrade);
        break;
    case commonHead::viewModels::model::TrayActionId::ShowAbout:
        activateMainWindow();
        sendUIEvent<UIAboutEvent>(UIAboutEvent::Action::ShowAboutDialog);
        break;
    case commonHead::viewModels::model::TrayActionId::Quit:
        sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Close);
        break;
    default:
        break;
    }
}

void MainWindowSystemTrayController::activateMainWindow()
{
    UIVIEW_LOG_DEBUG("MainWindowSystemTrayController::activateMainWindow");
    sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Show);
    sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Activate);
}

void MainWindowSystemTrayController::init()
{
    connect(m_viewModelEmitter.get(), &UIViewModelSignalBridge::TrayViewModelEmitter::signals_onMenuChanged,
            this, &MainWindowSystemTrayController::menuItemsChanged);
    connect(m_viewModelEmitter.get(), &UIViewModelSignalBridge::TrayViewModelEmitter::signals_onActionRequested,
            this, &MainWindowSystemTrayController::onActionRequested);

    m_trayViewModel = getViewModelFactory()->createTrayViewModelInstance();
    if (!m_trayViewModel)
    {
        UIVIEW_LOG_ERROR("failed to create TrayViewModel");
        return;
    }

    m_trayViewModel->registerCallback(m_viewModelEmitter);
    m_trayViewModel->initViewModel();
}

void MainWindowSystemTrayController::onLanguageChanged()
{
    if (m_trayViewModel)
        m_trayViewModel->reloadMenu();
}
