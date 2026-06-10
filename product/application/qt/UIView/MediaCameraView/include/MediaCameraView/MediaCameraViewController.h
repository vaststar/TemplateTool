#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include <QImage>
#include <QVideoFrame>
#include <QVideoSink>
#include <commonHead/viewModels/MediaCameraViewModel/VideoFrame.h>

#include "UIViewBase/UIViewController.h"
#include "ViewModelSingalEmitter/MediaCameraViewModelEmitter.h"

namespace commonHead{
    namespace viewModels{
        class IMediaCameraViewModel;
    }
    namespace model {
        struct VideoFrame;
    }
}

class AppContext;
class MediaCameraViewController: public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ getVideoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(bool visible READ isVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool openFailed READ isOpenFailed NOTIFY openFailedChanged)
    Q_PROPERTY(bool isOpening READ isOpening NOTIFY isOpeningChanged)
    QML_ELEMENT
public:
    explicit MediaCameraViewController(QObject *parent = nullptr);
    ~MediaCameraViewController();

    QVideoSink* getVideoSink() const;
    void setVideoSink(QVideoSink* videoSink);

    QVideoFrame imageToVideoFrame(const QImage& image) const;

    bool isVisible() const;
    bool isOpenFailed() const;
    bool isOpening() const;

    // Camera lifecycle. One instance == one camera; create a new controller
    // to bind a different camera.
    Q_INVOKABLE void openLocalCamera(int index);
    Q_INVOKABLE void openNetworkCamera(const QString& url,
                                       const QString& transport,
                                       int openTimeoutMs,
                                       int readTimeoutMs);
    Q_INVOKABLE void closeCamera();

private slots:
    virtual void onCameraFrameReceived(const commonHead::viewModels::model::VideoFrame& frame);
    virtual void onCameraOpenFailed();
signals:
    void showCameraImage(const QImage& image);
    void videoSinkChanged(QVideoSink* videoSink);
    void visibleChanged();
    void openFailedChanged();
    void isOpeningChanged();
protected:
    virtual void init() override;
private:
    std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> mMediaCameraViewModel;
    std::shared_ptr<UIVMSignalEmitter::MediaCameraViewModelEmitter>  mMediaCameraViewModelEmitter;
    QVideoSink* mVideoSink = nullptr;
    bool mOpenFailed = false;
    bool mIsOpening = false;
};
