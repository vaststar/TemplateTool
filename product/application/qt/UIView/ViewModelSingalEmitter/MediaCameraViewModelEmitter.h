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

    virtual void onCameraImageReceived(const commonHead::viewModels::model::Image& image) override {
        emit signals_onCameraImageReceived(image);
    };

signals:
    void signals_onCameraImageReceived(const commonHead::viewModels::model::Image& image);
};
}
