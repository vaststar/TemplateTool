#pragma once

#include <QObject>

#include <commonhead/viewmodels/ChatViewModel/IChatViewModel.h>
#include <UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h>

namespace UIViewModelSignalBridge {

class UIViewModelSignalBridge_EXPORT ChatViewModelEmitter final
    : public QObject
    , public commonHead::viewModels::IChatViewModelCallback
{
    Q_OBJECT

public:
    using QObject::QObject;

    void onChatChanged() override { emit chatChanged(); }

signals:
    void chatChanged();
};

} // namespace UIViewModelSignalBridge
