#pragma once

#include <QObject>
#include <commonHead/viewModels/StabilityViewModel/IStabilityViewModel.h>

namespace UIVMSignalEmitter{

class StabilityViewModelEmitter: public QObject,
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

} // namespace UIVMSignalEmitter
