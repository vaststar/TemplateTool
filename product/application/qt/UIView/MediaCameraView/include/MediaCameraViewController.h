#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>
#include <QImage>
#include <QVideoFrame>
#include <QVideosink>
#include <UICore/CoreController.h>
#include <commonHead/viewModels/ViewModelDataDefine/Image.h>

namespace commonHead{
    namespace viewModels{
        class IMediaCameraViewModel;
    }
    namespace model {
        struct Image;
    }
}

class AppContext;
class MediaCameraViewModelEmitter;
class MediaCameraViewController: public UICore::CoreController
{
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ getVideoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    QML_ELEMENT
public:
    explicit MediaCameraViewController(QObject *parent = nullptr);
    ~MediaCameraViewController();
    virtual QString getControllerName() const override;
    void initializeController(QPointer<AppContext> appContext);

    QVideoSink* getVideoSink() const;
    void setVideoSink(QVideoSink* videoSink);

    QVideoFrame imageToVideoFrame(const QImage& image) const;
private slots:
    virtual void onCameraImageReceived(const commonHead::viewModels::model::Image& image);
signals:
    void showCameraImage(const QImage& image);
    void videoSinkChanged(QVideoSink* videoSink);
private:
    QPointer<AppContext> mAppContext;
    std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> mMediaCameraViewModel;
    std::shared_ptr<MediaCameraViewModelEmitter>  mMediaCameraViewModelEmitter;
    QVideoSink* mVideoSink = nullptr;
};