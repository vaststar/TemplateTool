#include "MediaCameraView/include/MediaCameraViewController.h"

#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>

#include "ViewModelSingalEmitter/MediaCameraViewModelEmitter.h"

#include "LoggerDefine/LoggerDefine.h"

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
    mMediaCameraViewModel = getAppContext()->getViewModelFactory()->createMediaCameraViewModelInstance();
    mMediaCameraViewModel->registerCallback(mMediaCameraViewModelEmitter);
    mMediaCameraViewModel->openCamera();
    mMediaCameraViewModel->startCaptureCameraVideo();
}

bool MediaCameraViewController::isVisible() const
{
    return getAppContext() != nullptr;
}

void MediaCameraViewController::onCameraFrameReceived(const commonHead::viewModels::model::VideoFrame& frame)
{
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
