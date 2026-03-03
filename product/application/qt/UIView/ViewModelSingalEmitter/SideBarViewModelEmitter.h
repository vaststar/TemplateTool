#pragma once

#include <QObject>
#include <QMetaType>
#include <mutex>
#include <commonHead/viewModels/SideBarViewModel/ISideBarViewModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::NavItemData)
Q_DECLARE_METATYPE(commonHead::viewModels::model::PageChangeEvent)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::NavItemData>)

namespace UIVMSignalEmitter {

class SideBarViewModelEmitter : public QObject,
                                public commonHead::viewModels::ISideBarViewModelCallback
{
    Q_OBJECT
public:
    explicit SideBarViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
        static std::once_flag s_registeredFlag;
        std::call_once(s_registeredFlag, []() {
            qRegisterMetaType<commonHead::viewModels::model::NavItemData>();
            qRegisterMetaType<commonHead::viewModels::model::PageChangeEvent>();
            qRegisterMetaType<std::vector<commonHead::viewModels::model::NavItemData>>();
        });
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

signals:
    void signals_onNavItemsChanged(const std::vector<commonHead::viewModels::model::NavItemData>& items);
    void signals_onCurrentPageChanged(const commonHead::viewModels::model::PageChangeEvent& event);
    void signals_onNavItemUpdated(const commonHead::viewModels::model::NavItemData& item);
};

} // namespace UIVMSignalEmitter
