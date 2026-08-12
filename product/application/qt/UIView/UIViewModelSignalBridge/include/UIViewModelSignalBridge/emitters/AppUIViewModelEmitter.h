#pragma once

#include <QObject>
#include <UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h>
#include <commonhead/viewmodels/AppUIViewModel/IAppUIViewModel.h>

namespace UIVMSignalEmitter{
class UIViewModelSignalBridge_EXPORT AppUIViewModelEmitter: public QObject,
                                                        public commonHead::viewModels::IAppUIViewModelCallback
{
    Q_OBJECT
public:
    AppUIViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
    }

    virtual void onAppConfigInitialized() override
    {
        emit signals_onAppConfigInitialized();
    };
signals:
    void signals_onAppConfigInitialized();
};
}
