#pragma once

#include "RegisterViewModelMetaTypes.h"

#include <QObject>

#include <commonHead/viewModels/MiniAppListViewModel/IMiniAppListViewModel.h>

namespace UIVMSignalEmitter {

// Qt-signal bridge for IMiniAppListViewModelCallback.
class MiniAppListViewModelEmitter
    : public QObject
    , public commonHead::viewModels::IMiniAppListViewModelCallback
{
    Q_OBJECT
public:
    explicit MiniAppListViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent) {}

    void onMiniAppListChanged() override
    { emit signals_onMiniAppListChanged(); }

signals:
    void signals_onMiniAppListChanged();
};

} // namespace UIVMSignalEmitter
