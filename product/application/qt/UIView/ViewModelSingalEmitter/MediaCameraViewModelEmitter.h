#pragma once

#include <QObject>
#include <QMetaType>
#include <mutex>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::VideoFrame)

namespace UIVMSignalEmitter{
class MediaCameraViewModelEmitter: public QObject,
                                   public commonHead::viewModels::IMediaCameraViewModelCallback
{
    Q_OBJECT
public:
    MediaCameraViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
        static std::once_flag s_registeredFlag;
        std::call_once(s_registeredFlag, []() {
            qRegisterMetaType<commonHead::viewModels::model::VideoFrame>();
        });
    }

    virtual void onCameraFrameReceived(const commonHead::viewModels::model::VideoFrame& frame) override {
        emit signals_onCameraFrameReceived(frame);
    };

signals:
    void signals_onCameraFrameReceived(const commonHead::viewModels::model::VideoFrame& frame);
};
}
