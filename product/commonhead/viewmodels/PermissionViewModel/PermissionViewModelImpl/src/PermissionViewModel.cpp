#include "PermissionViewModel.h"
#include "LoggerDefine.h"

#include <stdexcept>
#include <commonhead/viewmodels/PermissionViewModel/PermissionViewModelCreator.h>
#include <ucf/utilities/PermissionPlatform/PermissionPlatform.h>

namespace commonHead::viewModels {
namespace {
namespace platform = ucf::utilities::permissions;

platform::Kind toPlatform(model::PermissionKind kind)
{
    switch (kind)
    {
    case model::PermissionKind::ScreenCapture: return platform::Kind::ScreenCapture;
    case model::PermissionKind::Microphone: return platform::Kind::Microphone;
    }
    throw std::invalid_argument("Invalid permission kind");
}

model::PermissionStatus toModel(platform::Status status)
{
    switch (status)
    {
    case platform::Status::Unknown: return model::PermissionStatus::Unknown;
    case platform::Status::NotDetermined: return model::PermissionStatus::NotDetermined;
    case platform::Status::NotGranted: return model::PermissionStatus::NotGranted;
    case platform::Status::Granted: return model::PermissionStatus::Granted;
    case platform::Status::Limited: return model::PermissionStatus::Limited;
    case platform::Status::Denied: return model::PermissionStatus::Denied;
    case platform::Status::Restricted: return model::PermissionStatus::Restricted;
    case platform::Status::Unsupported: return model::PermissionStatus::Unsupported;
    }
    return model::PermissionStatus::Unknown;
}

model::PermissionAction nextAction(const platform::State& state, bool requestAttempted)
{
    using Action = model::PermissionAction;
    switch (state.status)
    {
    case platform::Status::Granted:
    case platform::Status::Limited:
    case platform::Status::Restricted:
    case platform::Status::Unsupported:
        return Action::None;
    case platform::Status::NotDetermined:
        if (state.capabilities.supportsRequest) return Action::Request;
        break;
    case platform::Status::NotGranted:
        if (!requestAttempted && state.capabilities.supportsRequest)
        {
            return Action::Request;
        }
        break;
    case platform::Status::Unknown:
    case platform::Status::Denied:
        break;
    }
    return state.capabilities.supportsSettings ? Action::OpenSettings : Action::None;
}

const char* statusName(platform::Status status)
{
    switch (status)
    {
    case platform::Status::Unknown: return "Unknown";
    case platform::Status::NotDetermined: return "NotDetermined";
    case platform::Status::NotGranted: return "NotGranted";
    case platform::Status::Granted: return "Granted";
    case platform::Status::Limited: return "Limited";
    case platform::Status::Denied: return "Denied";
    case platform::Status::Restricted: return "Restricted";
    case platform::Status::Unsupported: return "Unsupported";
    }
    return "Unknown";
}

model::PermissionError actionError(platform::Error error, model::PermissionError failure)
{
    switch (error)
    {
    case platform::Error::None: return model::PermissionError::None;
    case platform::Error::Unsupported: return model::PermissionError::Unsupported;
    case platform::Error::PlatformFailure: return failure;
    }
    return failure;
}
} // namespace

namespace impl {
std::shared_ptr<IPermissionViewModel> createPermissionViewModel(
    commonHead::ICommonHeadFrameworkWptr framework, model::PermissionKind kind)
{
    return std::make_shared<PermissionViewModel>(framework, kind);
}
} // namespace impl

PermissionViewModel::PermissionViewModel(commonHead::ICommonHeadFrameworkWptr framework,
                                         model::PermissionKind kind)
    : IPermissionViewModel(framework)
    , mKind(kind)
{
    (void)toPlatform(kind);
}

std::string PermissionViewModel::getViewModelName() const
{
    return "PermissionViewModel";
}

void PermissionViewModel::init()
{
    refresh();
}

model::PermissionState PermissionViewModel::getState() const
{
    std::lock_guard lock(mMutex);
    return mState;
}

void PermissionViewModel::refresh()
{
    {
        std::lock_guard lock(mMutex);
        if (mState.busy) return;
        // Reserve the same guard used by native actions. No OS calls hold mMutex.
        mState.busy = true;
    }
    finishAction(model::PermissionError::None);
}

void PermissionViewModel::performAction()
{
    refresh();
    model::PermissionAction action;
    {
        std::lock_guard lock(mMutex);
        if (mState.busy || mState.action == model::PermissionAction::None) return;
        action = mState.action;
        mState.busy = true;
        mState.error = model::PermissionError::None;
        if (action == model::PermissionAction::Request) mRequestAttempted = true;
    }
    publishState();

    PERMISSION_VIEW_MODEL_LOG_INFO("Permission action started, kind="
        << (mKind == model::PermissionKind::ScreenCapture ? "ScreenCapture" : "Microphone")
        << ", action=" << (action == model::PermissionAction::Request ? "Request" : "OpenSettings"));
    const auto weakSelf = weak_from_this();
    try
    {
        if (action == model::PermissionAction::Request)
        {
            platform::request(toPlatform(mKind), [weakSelf](platform::State, platform::Error error) {
                if (auto self = weakSelf.lock())
                {
                    self->finishAction(actionError(error, model::PermissionError::RequestFailed),
                                       model::PermissionAction::Request);
                }
            });
        }
        else
        {
            platform::openSettings(toPlatform(mKind), [weakSelf](platform::Error error) {
                if (auto self = weakSelf.lock())
                {
                    self->finishAction(actionError(error, model::PermissionError::SettingsOpenFailed),
                                       model::PermissionAction::OpenSettings);
                }
            });
        }
    }
    catch (...)
    {
        finishAction(action == model::PermissionAction::Request
            ? model::PermissionError::RequestFailed : model::PermissionError::SettingsOpenFailed, action);
    }
}

void PermissionViewModel::finishAction(model::PermissionError error, model::PermissionAction action)
{
    platform::State native;
    try
    {
        native = platform::query(toPlatform(mKind));
        if (native.status == platform::Status::Unknown && error == model::PermissionError::None)
        {
            error = model::PermissionError::QueryFailed;
        }
    }
    catch (...)
    {
        if (error == model::PermissionError::None) error = model::PermissionError::QueryFailed;
    }
    {
        std::lock_guard lock(mMutex);
        if (native.status == platform::Status::Granted || native.status == platform::Status::Limited)
        {
            mRequestAttempted = false;
        }
        mState = {toModel(native.status), nextAction(native, mRequestAttempted), false, error};
    }
    if (action != model::PermissionAction::None)
    {
        PERMISSION_VIEW_MODEL_LOG_INFO("Permission action completed, kind="
            << (mKind == model::PermissionKind::ScreenCapture ? "ScreenCapture" : "Microphone")
            << ", action=" << (action == model::PermissionAction::Request ? "Request" : "OpenSettings")
            << ", result=" << (error == model::PermissionError::None ? "completed" : "failed")
            << ", status=" << statusName(native.status));
    }
    publishState();
}

void PermissionViewModel::publishState() noexcept
{
    try
    {
        const auto state = getState();
        fireNotification(&IPermissionViewModelCallback::onPermissionStateChanged, state);
    }
    catch (...)
    {
        // Observers must not unwind through an Objective-C/native permission callback.
    }
}

} // namespace commonHead::viewModels
