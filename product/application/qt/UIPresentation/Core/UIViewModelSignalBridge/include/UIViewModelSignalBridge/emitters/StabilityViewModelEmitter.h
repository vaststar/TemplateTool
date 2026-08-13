#pragma once

#include <QObject>
#include <UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h>
#include <commonhead/viewmodels/StabilityViewModel/IStabilityViewModel.h>

namespace UIViewModelSignalBridge{

class UIViewModelSignalBridge_EXPORT StabilityViewModelEmitter: public QObject,
                                                        public commonHead::viewModels::IStabilityViewModelCallback
{
    Q_OBJECT
public:
    StabilityViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }
    // 目前没有callback需要发射，预留扩展
};

} // namespace UIViewModelSignalBridge
