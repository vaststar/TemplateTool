#pragma once

#include <QObject>

#include <commonhead/viewmodels/TrayViewModel/ITrayViewModel.h>
#include <UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h>

namespace UIViewModelSignalBridge {

class UIViewModelSignalBridge_EXPORT TrayViewModelEmitter
    : public QObject
    , public commonHead::viewModels::ITrayViewModelCallback
{
    Q_OBJECT
public:
    explicit TrayViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    void onMenuChanged() override
    {
        emit signals_onMenuChanged();
    }

    void onActionRequested(commonHead::viewModels::model::TrayActionId actionId) override
    {
        emit signals_onActionRequested(static_cast<int>(actionId));
    }

signals:
    void signals_onMenuChanged();
    void signals_onActionRequested(int actionId);
};

} // namespace UIViewModelSignalBridge
