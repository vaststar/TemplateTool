#pragma once

#include <cstdint>
#include <mutex>
#include <string>

#include <ucf/Agents/MiniAppRuntimeAgent/IMiniAppRuntimeAgent.h>
#include <ucf/Agents/MiniAppRuntimeAgent/IMiniAppRuntimeAgentCallback.h>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/MiniAppRuntimeViewModel/IMiniAppRuntimeViewModel.h>

namespace ucf::service {
    class IMiniAppService;
    class IClientInfoService;
}

namespace commonHead::viewModels{
class MiniAppRuntimeViewModel: public virtual IMiniAppRuntimeViewModel,
                               public virtual commonHead::utilities::VMNotificationHelper<IMiniAppRuntimeViewModelCallback>,
                               public ucf::agents::IMiniAppRuntimeAgentCallback,
                               public std::enable_shared_from_this<MiniAppRuntimeViewModel>
{
public:
    explicit MiniAppRuntimeViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    MiniAppRuntimeViewModel(const MiniAppRuntimeViewModel&) = delete;
    MiniAppRuntimeViewModel(MiniAppRuntimeViewModel&&) = delete;
    MiniAppRuntimeViewModel& operator=(const MiniAppRuntimeViewModel&) = delete;
    MiniAppRuntimeViewModel& operator=(MiniAppRuntimeViewModel&&) = delete;
    ~MiniAppRuntimeViewModel() override;

public:
    std::string getViewModelName() const override;
    bool start(const std::string& appId) override;
    void stop() override;
    bool isReady() const override;
    std::uintptr_t nativeHostHandle() const override;

protected:
    void init() override;

    // IMiniAppRuntimeAgentCallback overrides — strip the framework/JSON layer
    // and forward as plain domain notifications to our own subscribers.
    void onReadyChanged(bool ready) override;
    void onLoadFinished(bool success) override;
    void onLoadFailed(int errorCode, const std::string& errorMessage) override;
    void onTitleChanged(const std::string& title) override;
    void onUrlChanged(const std::string& url) override;

private:
    std::shared_ptr<ucf::service::IMiniAppService> lockMiniAppService() const;
    std::shared_ptr<ucf::service::IClientInfoService> lockClientInfoService() const;

    mutable std::mutex mMutex;
    std::shared_ptr<ucf::agents::IMiniAppRuntimeAgent> mAgent;
    std::string mAppId;
};
}
