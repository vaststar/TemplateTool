#include "MediaCameraView/MediaCameraViewController.h"

#include <cstring>

#include <commonhead/viewmodels/MediaCameraViewModel/IMediaCameraViewModel.h>
#include <commonhead/viewmodels/MediaCameraViewModel/CameraSource.h>

#include <AppContext/AppContext.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>

#include <UIViewModelSignalBridge/emitters/MediaCameraViewModelEmitter.h>

#include "LoggerDefine.h"

MediaCameraViewController::MediaCameraViewController(QObject *parent)
    : UIViewController(parent)
    , mMediaCameraViewModelEmitter(std::make_shared<UIViewModelSignalBridge::MediaCameraViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("MediaCameraViewController constructed, address: " << this);
}

MediaCameraViewController::~MediaCameraViewController()
{
    UIVIEW_LOG_DEBUG("MediaCameraViewController destroying, address: " << this);
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
    QObject::connect(mMediaCameraViewModelEmitter.get(), &UIViewModelSignalBridge::MediaCameraViewModelEmitter::signals_onCameraFrameReceived, this, &MediaCameraViewController::onCameraFrameReceived, Qt::QueuedConnection);
    QObject::connect(mMediaCameraViewModelEmitter.get(), &UIViewModelSignalBridge::MediaCameraViewModelEmitter::signals_onCameraOpenFailed, this, &MediaCameraViewController::onCameraOpenFailed, Qt::QueuedConnection);
    mMediaCameraViewModel = getViewModelFactory()->createMediaCameraViewModelInstance();
    mMediaCameraViewModel->registerCallback(mMediaCameraViewModelEmitter);
    mMediaCameraViewModel->initViewModel();
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
    mMediaCameraViewModel->openCamera(
        commonHead::viewModels::model::LocalCameraSource{index});
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
    mMediaCameraViewModel->openCamera(source);
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
    mMediaCameraViewModel->closeCamera();
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
    if (!frame.isValid() ||
        frame.format != commonHead::viewModels::model::PixelFormat::RGB888)
    {
        return;
    }

    if (mIsOpening)
    {
        mIsOpening = false;
        emit isOpeningChanged();
    }

    // imageView does not own frame.data and therefore must not escape this
    // function. imageToVideoFrame() copies it into QVideoFrame-owned storage.
    const QImage imageView(frame.data.data(),
                           frame.width,
                           frame.height,
                           frame.bytesPerLine,
                           QImage::Format_RGB888);

    if (mVideoSink)
    {
        auto videoFrame = imageToVideoFrame(imageView);
        if (videoFrame.isValid())
        {
            mVideoSink->setVideoFrame(videoFrame);
        }
    }

    emit visibleChanged();
}

QVideoFrame MediaCameraViewController::imageToVideoFrame(const QImage& image) const
{
    if (image.isNull())
    {
        return {};
    }

    const QImage convertedImage = image.convertToFormat(QImage::Format_RGBX8888);
    if (convertedImage.isNull())
    {
        return {};
    }

    QVideoFrame videoFrame = QVideoFrameFormat(
        convertedImage.size(), QVideoFrameFormat::Format_RGBX8888);
    if (!videoFrame.map(QVideoFrame::WriteOnly))
    {
        return {};
    }

    const auto sourceBytesPerLine = convertedImage.bytesPerLine();
    const auto destinationBytesPerLine = videoFrame.bytesPerLine(0);
    const auto requiredDestinationBytes =
        static_cast<qint64>(convertedImage.height() - 1) * destinationBytesPerLine +
        sourceBytesPerLine;
    auto* destination = videoFrame.bits(0);
    if (!destination ||
        destinationBytesPerLine < sourceBytesPerLine ||
        requiredDestinationBytes > videoFrame.mappedBytes(0))
    {
        videoFrame.unmap();
        return {};
    }

    for (int row = 0; row < convertedImage.height(); ++row)
    {
        std::memcpy(destination + row * destinationBytesPerLine,
                    convertedImage.constScanLine(row),
                    static_cast<std::size_t>(sourceBytesPerLine));
    }

    videoFrame.unmap();
    return videoFrame;
}
