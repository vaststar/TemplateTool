#pragma once

#include <QObject>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

class MediaCameraViewModelEmitter: public QObject,
                                   public commonHead::viewModels::IMediaCameraViewModelCallback
{
    Q_OBJECT
public:
    MediaCameraViewModelEmitter(QObject* parent = nullptr)
        :QObject(parent)
    {
        qRegisterMetaType<commonHead::viewModels::model::Image>("commonHead::viewModels::model::Image");
    }

    virtual void onCameraImageReceived(const commonHead::viewModels::model::Image& image) override {
        emit signals_onCameraImageReceived(image);
    };

signals:
    void signals_onCameraImageReceived(const commonHead::viewModels::model::Image& image);
};
