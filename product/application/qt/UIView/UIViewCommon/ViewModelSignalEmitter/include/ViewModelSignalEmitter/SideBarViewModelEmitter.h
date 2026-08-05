#pragma once

#include <QObject>
#include <commonHead/viewModels/SideBarViewModel/ISideBarViewModel.h>
#include "RegisterViewModelMetaTypes.h"

namespace UIVMSignalEmitter {

class SideBarViewModelEmitter : public QObject,
                                public commonHead::viewModels::ISideBarViewModelCallback
{
    Q_OBJECT
public:
    explicit SideBarViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    void onSideBarReady() override
    {
        emit signals_onSideBarReady();
    }

    void onNavItemsAdded(const std::vector<commonHead::viewModels::model::NavItemData>& items) override
    {
        emit signals_onNavItemsAdded(items);
    }

    void onNavItemsUpdated(const std::vector<commonHead::viewModels::model::NavItemData>& items) override
    {
        emit signals_onNavItemsUpdated(items);
    }

    void onNavItemsRemoved(const std::vector<commonHead::viewModels::model::PageId>& pageIds) override
    {
        emit signals_onNavItemsRemoved(pageIds);
    }

    void onSubMenuRequested(commonHead::viewModels::model::PageId pageId,
                            const std::vector<commonHead::viewModels::model::SubMenuItem>& items) override
    {
        emit signals_onSubMenuRequested(static_cast<int>(pageId), items);
    }

    void onMenuActionClicked(commonHead::viewModels::model::MenuActionId actionId) override
    {
        emit signals_onMenuActionClicked(actionId);
    }

signals:
    void signals_onSideBarReady();
    void signals_onNavItemsAdded(const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void signals_onNavItemsUpdated(const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void signals_onNavItemsRemoved(const std::vector<commonHead::viewModels::model::PageId>& pageIds);
    void signals_onSubMenuRequested(int pageId, const std::vector<commonHead::viewModels::model::SubMenuItem>& items);
    void signals_onMenuActionClicked(commonHead::viewModels::model::MenuActionId actionId);
};

} // namespace UIVMSignalEmitter
