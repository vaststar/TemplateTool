#pragma once

#include <QObject>
#include <commonhead/viewmodels/MediaCameraViewModel/IMediaCameraViewModel.h>
#include <UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h>
#include <UIViewModelSignalBridge/metatypes/MediaCameraMetaTypes.h>

namespace UIViewModelSignalBridge{
class UIViewModelSignalBridge_EXPORT MediaCameraViewModelEmitter: public QObject,
                                                              public commonHead::viewModels::IMediaCameraViewModelCallback
{
    Q_OBJECT
public:
    MediaCameraViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
    }

    virtual void onCameraFrameReceived(const commonHead::viewModels::model::VideoFrame& frame) override {
        emit signals_onCameraFrameReceived(frame);
    };

    virtual void onCameraOpenFailed() override {
        emit signals_onCameraOpenFailed();
    };

signals:
    void signals_onCameraFrameReceived(const commonHead::viewModels::model::VideoFrame& frame);
    void signals_onCameraOpenFailed();
};
}
