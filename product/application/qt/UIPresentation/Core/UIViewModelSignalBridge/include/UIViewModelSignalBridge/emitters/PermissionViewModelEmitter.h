#pragma once

#include <QObject>
#include <UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h>
#include <UIViewModelSignalBridge/metatypes/PermissionMetaTypes.h>
#include <commonhead/viewmodels/PermissionViewModel/IPermissionViewModel.h>

namespace UIViewModelSignalBridge {

class UIViewModelSignalBridge_EXPORT PermissionViewModelEmitter
    : public QObject
    , public commonHead::viewModels::IPermissionViewModelCallback
{
    Q_OBJECT
public:
    explicit PermissionViewModelEmitter(QObject* parent = nullptr) : QObject(parent) {}

    void onPermissionStateChanged(const commonHead::viewModels::model::PermissionState& state) override
    {
        emit signals_onPermissionStateChanged(state);
    }

signals:
    void signals_onPermissionStateChanged(const commonHead::viewModels::model::PermissionState& state);
};

} // namespace UIViewModelSignalBridge
