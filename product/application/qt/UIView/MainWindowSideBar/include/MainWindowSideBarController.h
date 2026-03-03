#pragma once

#include "MainWindowSideBar/include/NavItemModel.h"
#include "ViewModelSingalEmitter/SideBarViewModelEmitter.h"

#include <QObject>
#include <QPointer>
#include <QtQml>

#include "UIViewBase/include/UIViewController.h"

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

signals:
    void currentPageIdChanged(int pageId);
    void pageChanged(int fromPageId, int toPageId, bool isUserAction);

protected:
    void init() override;

private slots:
    void onNavItemsChanged(const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void onCurrentPageChanged(const commonHead::viewModels::model::PageChangeEvent& event);
    void onNavItemUpdated(const commonHead::viewModels::model::NavItemData& item);

private:
    void refreshNavItems();

    std::shared_ptr<commonHead::viewModels::ISideBarViewModel> m_sideBarViewModel;
    std::shared_ptr<UIVMSignalEmitter::SideBarViewModelEmitter> m_viewModelEmitter;

    NavItemModel* m_topNavItems = nullptr;
    NavItemModel* m_bottomNavItems = nullptr;
    int m_currentPageId = 0;
};