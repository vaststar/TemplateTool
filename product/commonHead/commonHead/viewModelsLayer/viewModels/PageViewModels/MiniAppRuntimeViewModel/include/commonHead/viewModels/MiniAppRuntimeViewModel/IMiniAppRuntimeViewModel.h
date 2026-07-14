#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

namespace commonHead::viewModels{

// Runtime lifecycle notifications for a single mini-app. Deliberately free of
// any JSON or web-view types: the view model translates the framework agent's
// low-level events into these plain domain callbacks so the UI never depends on
// framework headers.
class COMMONHEAD_EXPORT IMiniAppRuntimeViewModelCallback
{
public:
    IMiniAppRuntimeViewModelCallback() = default;
    IMiniAppRuntimeViewModelCallback(const IMiniAppRuntimeViewModelCallback&) = delete;
    IMiniAppRuntimeViewModelCallback(IMiniAppRuntimeViewModelCallback&&) = delete;
    IMiniAppRuntimeViewModelCallback& operator=(const IMiniAppRuntimeViewModelCallback&) = delete;
    IMiniAppRuntimeViewModelCallback& operator=(IMiniAppRuntimeViewModelCallback&&) = delete;
    virtual ~IMiniAppRuntimeViewModelCallback() = default;

    virtual void onReadyChanged(bool ready) {}
    virtual void onLoadFinished(bool success) {}
    virtual void onLoadFailed(int errorCode, const std::string& errorMessage) {}
    virtual void onTitleChanged(const std::string& title) {}
    virtual void onUrlChanged(const std::string& url) {}
};

// View model that owns the runtime of one mini-app. It hides the framework
// runtime agent (web view, bridge, JSON) entirely: the UI hands it an app id,
// the view model resolves the package/permissions from the mini-app service,
// builds the agent, wires the bridge handlers, and exposes only a plain native
// window handle for embedding.
class COMMONHEAD_EXPORT IMiniAppRuntimeViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<IMiniAppRuntimeViewModelCallback>
{
public:
    using IViewModel::IViewModel;

    virtual void start(const std::string& appId) = 0;

    // Tear down the runtime. Safe to call more than once.
    virtual void stop() = 0;

    [[nodiscard]] virtual bool isReady() const = 0;

    // Native web-view window handle for embedding by the UI layer, or 0 if the
    // runtime has not started or has no embeddable backend. Exposed as a plain
    // integer so the view model interface stays free of platform/Qt types.
    [[nodiscard]] virtual std::uintptr_t nativeHostHandle() const = 0;

public:
    static std::shared_ptr<IMiniAppRuntimeViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}
