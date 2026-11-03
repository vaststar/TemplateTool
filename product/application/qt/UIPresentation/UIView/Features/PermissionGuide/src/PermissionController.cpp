#include <PermissionGuide/PermissionController.h>

#include <commonhead/viewmodels/PermissionViewModel/IPermissionViewModel.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <UIViewModelSignalBridge/emitters/PermissionViewModelEmitter.h>

#include <QMetaEnum>

#include "LoggerDefine.h"

namespace UIView {
namespace {
namespace model = commonHead::viewModels::model;

const char* permissionName(model::PermissionKind kind)
{
    switch (kind) {
    case model::PermissionKind::ScreenCapture: return "ScreenCapture";
    case model::PermissionKind::Microphone: return "Microphone";
    }
    return "Unknown";
}

bool sameState(const model::PermissionState& left, const model::PermissionState& right)
{
    return left.status == right.status && left.action == right.action
        && left.busy == right.busy && left.error == right.error;
}
} // namespace

PermissionController::PermissionController(model::PermissionKind kind, QObject* parent)
    : UIViewController(parent)
    , mKind(kind)
    , mEmitter(std::make_shared<UIViewModelSignalBridge::PermissionViewModelEmitter>())
{
    connect(mEmitter.get(),
        &UIViewModelSignalBridge::PermissionViewModelEmitter::signals_onPermissionStateChanged,
        this, [this](const model::PermissionState&) { synchronize(); }, Qt::QueuedConnection);
}

PermissionController::~PermissionController()
{
    if (mViewModel) mViewModel->unRegisterCallback(mEmitter);
}

void PermissionController::init()
{
    const auto factory = getViewModelFactory();
    if (!factory) return;
    mViewModel = factory->createPermissionViewModelInstance(mKind);
    if (!mViewModel) return;
    mViewModel->registerCallback(mEmitter);
    mViewModel->initViewModel();

    if (mActive) refresh();
    else synchronize();
}

PermissionController::Status PermissionController::status() const
{
    switch (mState.status) {
    case model::PermissionStatus::Unknown: return Status::Unknown;
    case model::PermissionStatus::NotDetermined: return Status::NotDetermined;
    case model::PermissionStatus::NotGranted: return Status::NotGranted;
    case model::PermissionStatus::Granted: return Status::Granted;
    case model::PermissionStatus::Limited: return Status::Limited;
    case model::PermissionStatus::Denied: return Status::Denied;
    case model::PermissionStatus::Restricted: return Status::Restricted;
    case model::PermissionStatus::Unsupported: return Status::Unsupported;
    }
    return Status::Unknown;
}

PermissionController::Action PermissionController::action() const
{
    switch (mState.action) {
    case model::PermissionAction::None: return Action::NoAction;
    case model::PermissionAction::Request: return Action::Request;
    case model::PermissionAction::OpenSettings: return Action::OpenSettings;
    }
    return Action::NoAction;
}

PermissionController::OperationError PermissionController::operationError() const
{
    switch (mState.error) {
    case model::PermissionError::None: return OperationError::NoError;
    case model::PermissionError::Unsupported: return OperationError::UnsupportedOperation;
    case model::PermissionError::QueryFailed: return OperationError::QueryFailed;
    case model::PermissionError::RequestFailed: return OperationError::RequestFailed;
    case model::PermissionError::SettingsOpenFailed: return OperationError::SettingsOpenFailed;
    }
    return OperationError::NoError;
}

bool PermissionController::granted() const { return status() == Status::Granted; }
bool PermissionController::needsGuidance() const
{
    return status() != Status::Granted && status() != Status::Limited
        && status() != Status::Unsupported;
}
bool PermissionController::isBusy() const { return mState.busy; }
bool PermissionController::isActive() const { return mActive; }

void PermissionController::setActive(bool active)
{
    if (mActive == active) return;
    mActive = active;
    emit activeChanged();
    if (mActive && mViewModel) refresh();
}

void PermissionController::refresh()
{
    if (!mViewModel) return;
    mViewModel->refresh();
    synchronize();
}

void PermissionController::performAction()
{
    if (!mViewModel) return;
    UIVIEW_LOG_INFO("Permission action requested, kind: " << permissionName(mKind)
        << ", action: " << QMetaEnum::fromType<Action>().valueToKey(static_cast<int>(action())));
    mViewModel->performAction();
    synchronize();
}

void PermissionController::synchronize()
{
    if (!mViewModel) return;
    // A queued notification may be old; project the ViewModel's current state.
    const auto state = mViewModel->getState();
    if (sameState(mState, state)) return;
    const bool wasGranted = granted();
    mState = state;
    UIVIEW_LOG_INFO("Permission state changed, kind: " << permissionName(mKind)
        << ", status: " << QMetaEnum::fromType<Status>().valueToKey(static_cast<int>(status()))
        << ", action: " << QMetaEnum::fromType<Action>().valueToKey(static_cast<int>(action()))
        << ", busy: " << isBusy()
        << ", error: " << QMetaEnum::fromType<OperationError>().valueToKey(static_cast<int>(operationError())));
    emit changed();
    if (wasGranted != granted()) emit grantedChanged();
}

} // namespace UIView
