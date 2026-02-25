#pragma once

#include <QObject>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

namespace UIVMSignalEmitter{
class MediaCameraViewModelEmitter: public QObject,
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

signals:
    void signals_onCameraFrameReceived(const commonHead::viewModels::model::VideoFrame& frame);
};
}
