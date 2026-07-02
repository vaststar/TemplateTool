#pragma once

#include <string>
#include <vector>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/MiniAppListViewModel/IMiniAppListViewModel.h>

namespace commonHead::viewModels{
class MiniAppListViewModel: public virtual IMiniAppListViewModel,
                            public virtual commonHead::utilities::VMNotificationHelper<IMiniAppListViewModelCallback>,
                            public std::enable_shared_from_this<MiniAppListViewModel>
{
public:
    explicit MiniAppListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    MiniAppListViewModel(const MiniAppListViewModel&) = delete;
    MiniAppListViewModel(MiniAppListViewModel&&) = delete;
    MiniAppListViewModel& operator=(const MiniAppListViewModel&) = delete;
    MiniAppListViewModel& operator=(MiniAppListViewModel&&) = delete;
    ~MiniAppListViewModel() = default;
public:
    virtual std::string getViewModelName() const override;
    virtual std::vector<commonHead::viewModels::model::MiniAppInfo> getMiniApps() const override;
    virtual commonHead::viewModels::model::MiniAppInfo getMiniApp(const std::string& id) const override;
protected:
    void init() override;
private:
    std::vector<commonHead::viewModels::model::MiniAppInfo> mMiniApps;
};
}
