#pragma once

#include "MainWindowSideBar/NavItemModel.h"
#include "ViewModelSingalEmitter/SideBarViewModelEmitter.h"

#include <QObject>
#include <QPointer>
#include <QtQml>

#include "UIViewBase/UIViewController.h"

class AppContext;

namespace commonHead::viewModels {
    class ISideBarViewModel;
}

namespace UIVMSignalEmitter {
    class SideBarViewModelEmitter;
}

// ==================== MainWindowSideBarController ====================

class MainWindowSideBarController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(NavItemModel* topNavItems READ topNavItems CONSTANT)
    Q_PROPERTY(NavItemModel* bottomNavItems READ bottomNavItems CONSTANT)
    Q_PROPERTY(int currentPageId READ currentPageId NOTIFY currentPageIdChanged)
    QML_ELEMENT

public:
    explicit MainWindowSideBarController(QObject* parent = nullptr);
    ~MainWindowSideBarController();

    NavItemModel* topNavItems() const;
    NavItemModel* bottomNavItems() const;
    int currentPageId() const;

    Q_INVOKABLE void navigateTo(int pageId);
    Q_INVOKABLE void handleSubMenuAction(int actionId);

signals:
    void currentPageIdChanged(int pageId);
    void pageChanged(int fromPageId, int toPageId, bool isUserAction);
    void subMenuRequested(int pageId, const QVariantList& menuItems);

protected:
    void init() override;
    void onLanguageChanged() override;

private slots:
    void onSideBarReady();
    void onNavItemsAdded  (const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void onNavItemsUpdated(const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void onNavItemsRemoved(const std::vector<commonHead::viewModels::model::PageId>& pageIds);
    void onSubMenuRequested(int pageId, const std::vector<commonHead::viewModels::model::SubMenuItem>& items);
    void onMenuActionClicked(commonHead::viewModels::model::MenuActionId actionId);

private:
    void seedFromSnapshot();
    void ensureValidSelection();
    void navigateToInternal(int pageId, bool isUserAction);

    std::shared_ptr<commonHead::viewModels::ISideBarViewModel> m_sideBarViewModel;
    std::shared_ptr<UIVMSignalEmitter::SideBarViewModelEmitter> m_viewModelEmitter;

    NavItemModel* m_topNavItems = nullptr;
    NavItemModel* m_bottomNavItems = nullptr;
    int m_currentPageId = 0;
};
