#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include <QImage>
#include <QVideoFrame>
#include <QVideosink>
#include <commonHead/viewModels/ViewModelDataDefine/Image.h>

#include "UIViewBase/include/UIViewController.h"

namespace commonHead{
    namespace viewModels{
        class IMediaCameraViewModel;
    }
    namespace model {
        struct Image;
    }
}

class AppContext;
namespace UIVMSignalEmitter{
class MediaCameraViewModelEmitter;
}
class MediaCameraViewController: public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ getVideoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(bool visible READ isVisible NOTIFY visibleChanged)
    QML_ELEMENT
public:
    explicit MediaCameraViewController(QObject *parent = nullptr);
    ~MediaCameraViewController();

    QVideoSink* getVideoSink() const;
    void setVideoSink(QVideoSink* videoSink);

    QVideoFrame imageToVideoFrame(const QImage& image) const;
    
    bool isVisible() const;
private slots:
    virtual void onCameraImageReceived(const commonHead::viewModels::model::Image& image);
signals:
    void showCameraImage(const QImage& image);
    void videoSinkChanged(QVideoSink* videoSink);
    void visibleChanged();
protected:
    virtual void init() override;
private:
    std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> mMediaCameraViewModel;
    std::shared_ptr<UIVMSignalEmitter::MediaCameraViewModelEmitter>  mMediaCameraViewModelEmitter;
    QVideoSink* mVideoSink = nullptr;
};