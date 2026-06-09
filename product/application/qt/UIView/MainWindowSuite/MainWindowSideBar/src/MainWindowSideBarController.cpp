#include "MainWindowSideBar/MainWindowSideBarController.h"

#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/SideBarViewModel/ISideBarViewModel.h>
#include <commonHead/viewModels/SideBarViewModel/SideBarModel.h>

#include "ViewModelSingalEmitter/SideBarViewModelEmitter.h"
#include "UIEvents/UIUpgradeEvent.h"
#include "UIEvents/UIAboutEvent.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

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
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onSideBarReady,
            this, &MainWindowSideBarController::onSideBarReady);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onNavItemsAdded,
            this, &MainWindowSideBarController::onNavItemsAdded);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onNavItemsUpdated,
            this, &MainWindowSideBarController::onNavItemsUpdated);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onNavItemsRemoved,
            this, &MainWindowSideBarController::onNavItemsRemoved);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onSubMenuRequested,
            this, &MainWindowSideBarController::onSubMenuRequested);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SideBarViewModelEmitter::signals_onMenuActionClicked,
            this, &MainWindowSideBarController::onMenuActionClicked);

    UIVIEW_LOG_DEBUG("MainWindowSideBarController::init signals connected, creating ViewModel");

    m_sideBarViewModel = getAppContext()->getViewModelFactory()->createSideBarViewModelInstance();
    if (!m_sideBarViewModel)
    {
        UIVIEW_LOG_ERROR("Failed to create SideBarViewModel!");
        return;
    }

    m_sideBarViewModel->registerCallback(m_viewModelEmitter);

    UIVIEW_LOG_DEBUG("MainWindowSideBarController::init ViewModel created, initializing");
    m_sideBarViewModel->initViewModel();
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
    navigateToInternal(pageId, /*isUserAction=*/true);
}

void MainWindowSideBarController::navigateToInternal(int pageId, bool isUserAction)
{
    if (!m_sideBarViewModel || pageId == m_currentPageId)
    {
        return;
    }

    // VM validation gate — false means "don't switch" (unknown/disabled/hidden/submenu).
    if (!m_sideBarViewModel->navigateTo(
            static_cast<commonHead::viewModels::model::PageId>(pageId), isUserAction))
    {
        return;
    }

    const int from = m_currentPageId;
    m_currentPageId = pageId;
    emit currentPageIdChanged(m_currentPageId);
    emit pageChanged(from, m_currentPageId, isUserAction);
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

void MainWindowSideBarController::seedFromSnapshot()
{
    if (!m_sideBarViewModel)
    {
        return;
    }

    m_topNavItems->resetItems(m_sideBarViewModel->getTopNavItems());
    m_bottomNavItems->resetItems(m_sideBarViewModel->getBottomNavItems());

    navigateToInternal(
        static_cast<int>(m_sideBarViewModel->getDefaultPageId()),
        /*isUserAction=*/false);

    ensureValidSelection();
}

void MainWindowSideBarController::ensureValidSelection()
{
    if (!m_sideBarViewModel)
    {
        return;
    }
    if (m_topNavItems->contains(m_currentPageId) || m_bottomNavItems->contains(m_currentPageId))
    {
        return;
    }
    navigateToInternal(
        static_cast<int>(m_sideBarViewModel->getDefaultPageId()),
        /*isUserAction=*/false);
}

void MainWindowSideBarController::onSideBarReady()
{
    UIVIEW_LOG_DEBUG("MainWindowSideBarController::onSideBarReady");
    seedFromSnapshot();
}

void MainWindowSideBarController::onNavItemsAdded(
    const std::vector<commonHead::viewModels::model::NavItemData>& items)
{
    if (items.empty())
    {
        return;
    }
    std::vector<commonHead::viewModels::model::NavItemData> top;
    std::vector<commonHead::viewModels::model::NavItemData> bottom;
    for (const auto& item : items)
    {
        (item.isTopItem() ? top : bottom).push_back(item);
    }
    if (!top.empty())    { m_topNavItems->insertItems(top); }
    if (!bottom.empty()) { m_bottomNavItems->insertItems(bottom); }
    ensureValidSelection();
}

void MainWindowSideBarController::onNavItemsUpdated(
    const std::vector<commonHead::viewModels::model::NavItemData>& items)
{
    if (items.empty())
    {
        return;
    }
    // Each model ignores items it doesn't own — hand the batch to both.
    m_topNavItems->updateItems(items);
    m_bottomNavItems->updateItems(items);
    ensureValidSelection();
}

void MainWindowSideBarController::onNavItemsRemoved(
    const std::vector<commonHead::viewModels::model::PageId>& pageIds)
{
    if (pageIds.empty())
    {
        return;
    }
    m_topNavItems->removeItems(pageIds);
    m_bottomNavItems->removeItems(pageIds);
    ensureValidSelection();
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
