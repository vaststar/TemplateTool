#include "MediaCameraView/include/MediaCameraViewController.h"

#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

#include <AppContext/AppContext.h>
#include <UIFabrication/IViewModelFactory.h>

#include "ViewModelSingalEmitter/MediaCameraViewModelEmitter.h"

#include "LoggerDefine/LoggerDefine.h"

MediaCameraViewController::MediaCameraViewController(QObject *parent)
    : CoreController(parent)
    , mAppContext(nullptr)
    , mMediaCameraViewModelEmitter( std::make_shared<MediaCameraViewModelEmitter>(this))
{
    UIVIEW_LOG_DEBUG("create MediaCameraViewController");
}

MediaCameraViewController::~MediaCameraViewController()
{
    UIVIEW_LOG_DEBUG("delete MediaCameraViewController");
}

QString MediaCameraViewController::getControllerName() const
{
    return QObject::tr("MediaCameraViewController");
}

void MediaCameraViewController::initializeController(QPointer<AppContext> appContext)
{
    mAppContext = appContext;
    QObject::connect(mMediaCameraViewModelEmitter.get(), &MediaCameraViewModelEmitter::signals_onCameraImageReceived, this, &MediaCameraViewController::onCameraImageReceived);
    mMediaCameraViewModel = mAppContext->getViewModelFactory()->createMediaCameraViewModelInstance();
    mMediaCameraViewModel->registerCallback(mMediaCameraViewModelEmitter);
    mMediaCameraViewModel->openCamera();
    mMediaCameraViewModel->startCaptureCameraVideo();
}

void MediaCameraViewController::onCameraImageReceived(const commonHead::viewModels::model::Image& image)
{
    QImage img(&image.buffer[0], image.width, image.height, static_cast<int>(image.steps), QImage::Format::Format_RGB888);
    

    QByteArray byteArray;
QBuffer buffer(&byteArray);
img.save(&buffer, "PNG");
mImageData = "data:image/png;base64," + byteArray.toBase64();
emit showCameraImage(img);
}
QString MediaCameraViewController::getImage()
{
    return mImageData;
}
