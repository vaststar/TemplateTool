#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/MiniAppListViewModel/IMiniAppListViewModel.h>
#include <ucf/services/MiniAppService/IMiniAppServiceCallback.h>

namespace ucf::service {
    class IMiniAppService;
}

namespace commonHead::viewModels {
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
    ~MiniAppListViewModel() override;
public:
    std::string getViewModelName() const override;
    std::vector<commonHead::viewModels::model::MiniAppInfo> getMiniApps() const override;
    commonHead::viewModels::model::MiniAppInfo getMiniApp(const std::string& id) const override;
    void installMiniApp(const std::string& sourceDirectory) override;
    void uninstallMiniApp(const std::string& id) override;
protected:
    void init() override;

    // IMiniAppServiceCallback overrides — apply the service delta, then notify
    // our own subscribers via onMiniAppListChanged() (or a failure callback).
    void onMiniAppServiceReady() override;
    void onMiniAppInstalled(const ucf::service::model::MiniAppManifest& app) override;
    void onMiniAppInstallFailed(ucf::service::MiniAppInstallError error) override;
    void onMiniAppUninstalled(const std::string& id) override;
    void onMiniAppUninstallFailed(ucf::service::MiniAppUninstallError error) override;
private:
    std::shared_ptr<ucf::service::IMiniAppService> lockService() const;
    // Rebuild the full in-memory list from the service snapshot.
    void rebuildFromService();

    // Resolve the service-layer error to localized (title, message) strings and
    // fire the corresponding failure callback.
    void notifyInstallFailed(ucf::service::MiniAppInstallError error);
    void notifyUninstallFailed(ucf::service::MiniAppUninstallError error);

    mutable std::mutex mMutex;
    std::vector<commonHead::viewModels::model::MiniAppInfo> mMiniApps;
};
} // namespace commonHead::viewModels
