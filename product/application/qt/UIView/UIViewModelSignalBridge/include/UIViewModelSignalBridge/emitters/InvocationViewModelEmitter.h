#pragma once

#include <QObject>
#include <UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h>
#include <commonhead/viewmodels/InvocationViewModel/IInvocationViewModel.h>

namespace UIVMSignalEmitter{
class UIViewModelSignalBridge_EXPORT InvocationViewModelEmitter: public QObject,
                                                        public commonHead::viewModels::IInvocationViewModelCallback
{
    Q_OBJECT
public:
    InvocationViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
    }

};
}
