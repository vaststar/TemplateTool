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

    void onNavItemsChanged(const std::vector<commonHead::viewModels::model::NavItemData>& items) override
    {
        emit signals_onNavItemsChanged(items);
    }

    void onCurrentPageChanged(const commonHead::viewModels::model::PageChangeEvent& event) override
    {
        emit signals_onCurrentPageChanged(event);
    }

    void onNavItemUpdated(const commonHead::viewModels::model::NavItemData& item) override
    {
        emit signals_onNavItemUpdated(item);
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
    void signals_onNavItemsChanged(const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void signals_onCurrentPageChanged(const commonHead::viewModels::model::PageChangeEvent& event);
    void signals_onNavItemUpdated(const commonHead::viewModels::model::NavItemData& item);
    void signals_onSubMenuRequested(int pageId, const std::vector<commonHead::viewModels::model::SubMenuItem>& items);
    void signals_onMenuActionClicked(commonHead::viewModels::model::MenuActionId actionId);
};

} // namespace UIVMSignalEmitter
