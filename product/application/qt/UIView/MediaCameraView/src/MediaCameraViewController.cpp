#include "MediaCameraView/MediaCameraViewController.h"

#include <thread>

#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>
#include <commonHead/viewModels/MediaCameraViewModel/CameraSource.h>

#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>

#include "ViewModelSingalEmitter/MediaCameraViewModelEmitter.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

MediaCameraViewController::MediaCameraViewController(QObject *parent)
    : UIViewController(parent)
    , mMediaCameraViewModelEmitter(std::make_shared<UIVMSignalEmitter::MediaCameraViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create MediaCameraViewController");
}

MediaCameraViewController::~MediaCameraViewController()
{
    UIVIEW_LOG_DEBUG("delete MediaCameraViewController");
}

QVideoSink* MediaCameraViewController::getVideoSink() const
{
    return mVideoSink;
}

void MediaCameraViewController::setVideoSink(QVideoSink* videoSink)
{
    if (!videoSink)
    {
        return;
    }

    if (mVideoSink != videoSink)
    {
        mVideoSink = videoSink;
        emit videoSinkChanged(mVideoSink);
    }
}
void MediaCameraViewController::init()
{
    UIVIEW_LOG_DEBUG("");
    QObject::connect(mMediaCameraViewModelEmitter.get(), &UIVMSignalEmitter::MediaCameraViewModelEmitter::signals_onCameraFrameReceived, this, &MediaCameraViewController::onCameraFrameReceived);
    QObject::connect(mMediaCameraViewModelEmitter.get(), &UIVMSignalEmitter::MediaCameraViewModelEmitter::signals_onCameraOpenFailed, this, &MediaCameraViewController::onCameraOpenFailed);
    mMediaCameraViewModel = getAppContext()->getViewModelFactory()->createMediaCameraViewModelInstance();
    mMediaCameraViewModel->registerCallback(mMediaCameraViewModelEmitter);
    // Callers drive the camera lifecycle via openLocal/openNetworkCamera.
}

void MediaCameraViewController::openLocalCamera(int index)
{
    if (!mMediaCameraViewModel)
    {
        UIVIEW_LOG_WARN("openLocalCamera called before init; ignoring");
        return;
    }
    UIVIEW_LOG_DEBUG("openLocalCamera index=" << index);
    if (mOpenFailed)
    {
        mOpenFailed = false;
        emit openFailedChanged();
    }
    if (!mIsOpening)
    {
        mIsOpening = true;
        emit isOpeningChanged();
    }
    auto vm = mMediaCameraViewModel;
    std::thread([vm, index] {
        vm->openCamera(commonHead::viewModels::model::LocalCameraSource{index});
        vm->startCaptureCameraVideo();
    }).detach();
}

void MediaCameraViewController::openNetworkCamera(const QString& url,
                                                  const QString& transport,
                                                  int openTimeoutMs,
                                                  int readTimeoutMs)
{
    if (!mMediaCameraViewModel)
    {
        UIVIEW_LOG_WARN("openNetworkCamera called before init; ignoring");
        return;
    }
    UIVIEW_LOG_DEBUG("openNetworkCamera url=" << url.toStdString());
    if (mOpenFailed)
    {
        mOpenFailed = false;
        emit openFailedChanged();
    }
    if (!mIsOpening)
    {
        mIsOpening = true;
        emit isOpeningChanged();
    }
    commonHead::viewModels::model::NetworkCameraSource source;
    source.url           = url.toStdString();
    source.transport     = transport.toStdString();
    source.openTimeoutMs = openTimeoutMs;
    source.readTimeoutMs = readTimeoutMs;
    auto vm = mMediaCameraViewModel;
    std::thread([vm, source = std::move(source)] {
        vm->openCamera(source);
        vm->startCaptureCameraVideo();
    }).detach();
}

void MediaCameraViewController::closeCamera()
{
    if (!mMediaCameraViewModel)
    {
        return;
    }
    UIVIEW_LOG_DEBUG("closeCamera");
    if (mIsOpening)
    {
        mIsOpening = false;
        emit isOpeningChanged();
    }
    auto vm = mMediaCameraViewModel;
    std::thread([vm] {
        vm->stopCaptureCameraVideo();
    }).detach();
}

bool MediaCameraViewController::isVisible() const
{
    return getAppContext() != nullptr;
}

bool MediaCameraViewController::isOpenFailed() const
{
    return mOpenFailed;
}

bool MediaCameraViewController::isOpening() const
{
    return mIsOpening;
}

void MediaCameraViewController::onCameraOpenFailed()
{
    UIVIEW_LOG_WARN("camera open failed");
    if (mIsOpening)
    {
        mIsOpening = false;
        emit isOpeningChanged();
    }
    if (!mOpenFailed)
    {
        mOpenFailed = true;
        emit openFailedChanged();
    }
}

void MediaCameraViewController::onCameraFrameReceived(const commonHead::viewModels::model::VideoFrame& frame)
{
    if (mIsOpening)
    {
        mIsOpening = false;
        emit isOpeningChanged();
    }
    QImage img(&frame.data[0], frame.width, frame.height, frame.bytesPerLine, QImage::Format::Format_RGB888);

    if(mVideoSink)
    {
        mVideoSink->setVideoFrame(imageToVideoFrame(img));
    }

    emit showCameraImage(img);

    emit visibleChanged();
}

QVideoFrame MediaCameraViewController::imageToVideoFrame(const QImage& image) const
{
    QImage convetedImage = image.convertToFormat(QImage::Format_RGBX8888);
    QVideoFrame videoFrame = QVideoFrameFormat(convetedImage.size(), QVideoFrameFormat::Format_RGBX8888);
    if (videoFrame.map(QVideoFrame::WriteOnly))
    {
        memcpy(videoFrame.bits(0), convetedImage.bits(), convetedImage.sizeInBytes());
        videoFrame.unmap();
        return videoFrame;
    }
    return {};
}
