#include "MainWindowSideBar/include/MainWindowSideBarController.h"

#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/SideBarViewModel/ISideBarViewModel.h>
#include <commonHead/viewModels/SideBarViewModel/SideBarModel.h>

#include "ViewModelSingalEmitter/SideBarViewModelEmitter.h"
#include "UIEvents/UIUpgradeEvent.h"
#include "UIEvents/UIAboutEvent.h"
#include "LoggerDefine/LoggerDefine.h"

// ==================== MainWindowSideBarController ====================

MainWindowSideBarController::MainWindowSideBarController(QObject* parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIVMSignalEmitter::SideBarViewModelEmitter>())
    , m_topNavItems(new NavItemModel(this))
    , m_bottomNavItems(new NavItemModel(this))
{
    UIVIEW_LOG_DEBUG("create MainWindowSideBarController");
}

MainWindowSideBarController::~MainWindowSideBarController()
{
    UIVIEW_LOG_DEBUG("destroy MainWindowSideBarController");
}

void MainWindowSideBarController::init()
{
    UIVIEW_LOG_DEBUG("MainWindowSideBarController::init start");

    // Connect signals from ViewModel emitter
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onNavItemsChanged,
            this, &MainWindowSideBarController::onNavItemsChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onCurrentPageChanged,
            this, &MainWindowSideBarController::onCurrentPageChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onNavItemUpdated,
            this, &MainWindowSideBarController::onNavItemUpdated);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onSubMenuRequested,
            this, &MainWindowSideBarController::onSubMenuRequested);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onMenuActionClicked,
            this, &MainWindowSideBarController::onMenuActionClicked);

    UIVIEW_LOG_DEBUG("MainWindowSideBarController::init signals connected, creating ViewModel");

    // Create ViewModel and register callback
    m_sideBarViewModel = getAppContext()->getViewModelFactory()->createSideBarViewModelInstance();
    if (!m_sideBarViewModel)
    {
        UIVIEW_LOG_ERROR("Failed to create SideBarViewModel!");
        return;
    }

    UIVIEW_LOG_DEBUG("MainWindowSideBarController::init ViewModel created, initializing");
    m_sideBarViewModel->initViewModel();

    UIVIEW_LOG_DEBUG("MainWindowSideBarController::init registering callback");
    m_sideBarViewModel->registerCallback(m_viewModelEmitter);

    // Load initial data
    UIVIEW_LOG_DEBUG("MainWindowSideBarController::init refreshing nav items");
    refreshNavItems();

    UIVIEW_LOG_DEBUG("MainWindowSideBarController::init done");
}

NavItemModel* MainWindowSideBarController::topNavItems() const
{
    return m_topNavItems;
}

NavItemModel* MainWindowSideBarController::bottomNavItems() const
{
    return m_bottomNavItems;
}

int MainWindowSideBarController::currentPageId() const
{
    return m_currentPageId;
}

void MainWindowSideBarController::navigateTo(int pageId)
{
    if (m_sideBarViewModel)
    {
        m_sideBarViewModel->navigateTo(static_cast<commonHead::viewModels::model::PageId>(pageId));
    }
}

void MainWindowSideBarController::handleSubMenuAction(int actionId)
{
    if (m_sideBarViewModel)
    {
        m_sideBarViewModel->handleSubMenuAction(
            static_cast<commonHead::viewModels::model::MenuActionId>(actionId));
    }
}

void MainWindowSideBarController::onMenuActionClicked(commonHead::viewModels::model::MenuActionId actionId)
{
    switch (actionId)
    {
    case commonHead::viewModels::model::MenuActionId::CheckUpgrade:
        sendUIEvent<UIUpgradeEvent>(UIUpgradeEvent::Action::CheckForUpgrade);
        break;
    case commonHead::viewModels::model::MenuActionId::About:
        sendUIEvent<UIAboutEvent>(UIAboutEvent::Action::ShowAboutDialog);
        break;
    default:
        break;
    }
}

void MainWindowSideBarController::refreshNavItems()
{
    if (!m_sideBarViewModel)
    {
        return;
    }

    m_topNavItems->setItems(m_sideBarViewModel->getTopNavItems());
    m_bottomNavItems->setItems(m_sideBarViewModel->getBottomNavItems());

    if (const int newPageId = static_cast<int>(m_sideBarViewModel->getCurrentPageId()); m_currentPageId != newPageId)
    {
        m_currentPageId = newPageId;
        emit currentPageIdChanged(m_currentPageId);
    }
}

void MainWindowSideBarController::onNavItemsChanged(const std::vector<commonHead::viewModels::model::NavItemData>& items)
{
    Q_UNUSED(items);
    refreshNavItems();
}

void MainWindowSideBarController::onCurrentPageChanged(const commonHead::viewModels::model::PageChangeEvent& event)
{
    m_currentPageId = static_cast<int>(event.toPageId);
    emit currentPageIdChanged(m_currentPageId);
    emit pageChanged(static_cast<int>(event.fromPageId), static_cast<int>(event.toPageId), event.isUserAction);
}

void MainWindowSideBarController::onNavItemUpdated(const commonHead::viewModels::model::NavItemData& item)
{
    if (item.isTopItem())
    {
        m_topNavItems->updateItem(item);
    }
    else
    {
        m_bottomNavItems->updateItem(item);
    }
}

void MainWindowSideBarController::onSubMenuRequested(
    int pageId, const std::vector<commonHead::viewModels::model::SubMenuItem>& items)
{
    QVariantList menuItems;
    for (const auto& item : items)
    {
        QVariantMap entry;
        entry["actionId"] = static_cast<int>(item.actionId);
        entry["text"] = QString::fromStdString(item.title);
        menuItems.append(entry);
    }
    emit subMenuRequested(pageId, menuItems);
}

void MainWindowSideBarController::onLanguageChanged()
{
    UIVIEW_LOG_DEBUG("MainWindowSideBarController::onLanguageChanged");
    if (m_sideBarViewModel)
    {
        m_sideBarViewModel->reloadNavConfig();
    }
}
