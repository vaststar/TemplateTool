#pragma once

#include <atomic>
#include <memory>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/AppUIViewModel/IAppUIViewModel.h>
#include <ucf/services/ClientInfoService/IClientInfoServiceCallback.h>

namespace commonHead::viewModels{
class AppUIViewModel: public virtual IAppUIViewModel,
                      public virtual commonHead::utilities::VMNotificationHelper<IAppUIViewModelCallback>,
                      public ucf::service::IClientInfoServiceCallback,
                      public std::enable_shared_from_this<AppUIViewModel>
{
public:
    explicit AppUIViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    AppUIViewModel(const AppUIViewModel&) = delete;
    AppUIViewModel(AppUIViewModel&&) = delete;
    AppUIViewModel& operator=(const AppUIViewModel&) = delete;
    AppUIViewModel& operator=(AppUIViewModel&&) = delete;
    ~AppUIViewModel() override;
public:
    virtual std::string getViewModelName() const override;

    // IClientInfoServiceCallback
    virtual void onClientInfoReady() override;
    void onClientInfoLoadFailed(ucf::service::ClientInfoLoadError error) override;
    virtual void onClientThemeChanged(ucf::service::model::ThemeType themeType) override;
protected:
    void init() override;
private:
    void publishInitializationFailure(const std::string& reason);

    // Service retries may notify again; this VM delivers one startup result.
    std::atomic<bool> mInitializationResultDelivered{false};
};
}
