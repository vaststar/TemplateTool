#pragma once

#include <QObject>
#include <commonHead/viewModels/InvocationViewModel/IInvocationViewModel.h>

namespace UIVMSignalEmitter{
class InvocationViewModelEmitter: public QObject,
                             public commonHead::viewModels::IInvocationViewModelCallback
{
    Q_OBJECT
public:
    InvocationViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
    }

    virtual void onActivateMainApp() override
    {
        emit signals_onActivateMainApp();
    };
signals:
    void signals_onActivateMainApp();
};
}
