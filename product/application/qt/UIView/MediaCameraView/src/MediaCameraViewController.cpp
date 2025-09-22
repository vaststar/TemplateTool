#include "MediaCameraView/include/MediaCameraViewController.h"

#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

#include <AppContext/AppContext.h>
#include <UIFabrication/IViewModelFactory.h>

#include "ViewModelSingalEmitter/MediaCameraViewModelEmitter.h"

#include "LoggerDefine/LoggerDefine.h"

MediaCameraViewController::MediaCameraViewController(QObject *parent)
    : UIViewController(parent)
    , mMediaCameraViewModelEmitter( std::make_shared<MediaCameraViewModelEmitter>(this))
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
    QObject::connect(mMediaCameraViewModelEmitter.get(), &MediaCameraViewModelEmitter::signals_onCameraImageReceived, this, &MediaCameraViewController::onCameraImageReceived);
    mMediaCameraViewModel = getAppContext()->getViewModelFactory()->createMediaCameraViewModelInstance();
    mMediaCameraViewModel->registerCallback(mMediaCameraViewModelEmitter);
    mMediaCameraViewModel->openCamera();
    mMediaCameraViewModel->startCaptureCameraVideo();
}

void MediaCameraViewController::onCameraImageReceived(const commonHead::viewModels::model::Image& image)
{
    QImage img(&image.buffer[0], image.width, image.height, static_cast<int>(image.steps), QImage::Format::Format_RGB888);
    
    if(mVideoSink)
    {
        mVideoSink->setVideoFrame(imageToVideoFrame(img));
    }

    emit showCameraImage(img);
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
