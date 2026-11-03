#pragma once

#include <memory>
#include <QtQml/qqmlregistration.h>

#include <UIViewCore/UIViewController.h>
#include <commonhead/viewmodels/PermissionViewModel/PermissionModel.h>

namespace commonHead::viewModels {
class IPermissionViewModel;
}
namespace UIViewModelSignalBridge {
class PermissionViewModelEmitter;
}

namespace UIView {

// Page-local Qt projection of one permission. The ViewModel owns its operation state.
class PermissionController final : public UIViewController
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PermissionController)
    QML_UNCREATABLE("Permission controllers are owned by feature controllers")

    Q_PROPERTY(Status status READ status NOTIFY changed)
    Q_PROPERTY(Action action READ action NOTIFY changed)
    Q_PROPERTY(OperationError operationError READ operationError NOTIFY changed)
    Q_PROPERTY(bool granted READ granted NOTIFY grantedChanged)
    Q_PROPERTY(bool needsGuidance READ needsGuidance NOTIFY changed)
    Q_PROPERTY(bool busy READ isBusy NOTIFY changed)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

public:
    enum class Status {
        Unknown, NotDetermined, NotGranted, Granted,
        Limited, Denied, Restricted, Unsupported
    };
    Q_ENUM(Status)

    enum class Action { NoAction, Request, OpenSettings };
    Q_ENUM(Action)

    enum class OperationError {
        NoError, UnsupportedOperation, QueryFailed, RequestFailed, SettingsOpenFailed
    };
    Q_ENUM(OperationError)

    explicit PermissionController(commonHead::viewModels::model::PermissionKind kind,
                                  QObject* parent);
    ~PermissionController() override;

    Status status() const;
    Action action() const;
    OperationError operationError() const;
    bool granted() const;
    bool needsGuidance() const;
    bool isBusy() const;
    bool isActive() const;
    void setActive(bool active);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void performAction();

signals:
    void changed();
    void grantedChanged();
    void activeChanged();

protected:
    void init() override;

private:
    void synchronize();

    const commonHead::viewModels::model::PermissionKind mKind;
    std::shared_ptr<commonHead::viewModels::IPermissionViewModel> mViewModel;
    std::shared_ptr<UIViewModelSignalBridge::PermissionViewModelEmitter> mEmitter;
    commonHead::viewModels::model::PermissionState mState;
    bool mActive = false;
};

} // namespace UIView
