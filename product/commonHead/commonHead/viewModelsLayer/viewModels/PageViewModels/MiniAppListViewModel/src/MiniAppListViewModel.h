#pragma once

#include <mutex>
#include <string>
#include <vector>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/MiniAppListViewModel/IMiniAppListViewModel.h>
#include <ucf/Services/MiniAppService/IMiniAppServiceCallback.h>

namespace ucf::service {
    class IMiniAppService;
}

namespace commonHead::viewModels{
class MiniAppListViewModel: public virtual IMiniAppListViewModel,
                            public virtual commonHead::utilities::VMNotificationHelper<IMiniAppListViewModelCallback>,
                            public ucf::service::IMiniAppServiceCallback,
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
    virtual bool installMiniApp(const std::string& sourceDirectory) override;
    virtual bool uninstallMiniApp(const std::string& id) override;
protected:
    void init() override;

    // IMiniAppServiceCallback overrides — apply the service delta, then notify
    // our own subscribers via onMiniAppListChanged().
    void onMiniAppServiceReady() override;
    void onMiniAppInstalled(const ucf::service::model::MiniAppManifest& app) override;
    void onMiniAppUninstalled(const std::string& id) override;
private:
    std::shared_ptr<ucf::service::IMiniAppService> lockService() const;
    // Rebuild the full in-memory list from the service snapshot.
    void rebuildFromService();

    mutable std::mutex mMutex;
    std::vector<commonHead::viewModels::model::MiniAppInfo> mMiniApps;
};
}
