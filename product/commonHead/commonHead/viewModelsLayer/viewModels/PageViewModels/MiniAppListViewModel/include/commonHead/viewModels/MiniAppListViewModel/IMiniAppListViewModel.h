#pragma once

#include <memory>
#include <string>
#include <vector>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/MiniAppListViewModel/MiniAppInfo.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IMiniAppListViewModelCallback
{
public:
    IMiniAppListViewModelCallback() = default;
    IMiniAppListViewModelCallback(const IMiniAppListViewModelCallback&) = delete;
    IMiniAppListViewModelCallback(IMiniAppListViewModelCallback&&) = delete;
    IMiniAppListViewModelCallback& operator=(const IMiniAppListViewModelCallback&) = delete;
    IMiniAppListViewModelCallback& operator=(IMiniAppListViewModelCallback&&) = delete;
    virtual ~IMiniAppListViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IMiniAppListViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IMiniAppListViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IMiniAppListViewModel(const IMiniAppListViewModel&) = delete;
    IMiniAppListViewModel(IMiniAppListViewModel&&) = delete;
    IMiniAppListViewModel& operator=(const IMiniAppListViewModel&) = delete;
    IMiniAppListViewModel& operator=(IMiniAppListViewModel&&) = delete;
    virtual ~IMiniAppListViewModel() = default;
public:
    virtual std::string getViewModelName() const = 0;

    // Returns the list of installed, launchable mini apps.
    virtual std::vector<commonHead::viewModels::model::MiniAppInfo> getMiniApps() const = 0;

    // Returns a single mini app by id, or an empty MiniAppInfo when not found.
    virtual commonHead::viewModels::model::MiniAppInfo getMiniApp(const std::string& id) const = 0;
public:
    static std::shared_ptr<IMiniAppListViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}
