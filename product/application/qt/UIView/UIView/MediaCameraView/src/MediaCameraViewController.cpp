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
    mMediaCameraViewModel->openCamera();
    mMediaCameraViewModel->startCaptureCameraVideo();
}

void MediaCameraViewController::onCameraImageReceived(const commonHead::viewModels::model::Image& image)
{

}
